/**
 * @file    Task.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: find how to unify all xxx-Task classes in one simple class
 * TODO: consider moving self/nextDependencies to the Node level 
 */

#include "Task.hpp"
#include "LocalTask.hpp"
#include "ScalarTask.hpp"
#include "FocalTask.hpp"
#include "ZonalTask.hpp"
#include "FocalZonalTask.hpp"
#include "RadiatingTask.hpp"
#include "SpreadingTask.hpp"
#include "StatsTask.hpp"
#include "../ThreadId.hpp"
#include "../Runtime.hpp"
#include <memory>
#include <cassert>


namespace map { namespace detail {

/********+*
   Utils
 **********/

std::size_t task_hash::operator()(const Task *t) const {
	return std::hash<const Task*>()(t);
}

/********
   Task
 ********/

Task* Task::Factory(Group *group) {
	Pattern pat = group->pattern();

	/**/ if ( pat.is(SPECIAL) )
	{
		if ( pat.is(ZONAL) )
		{
			return new StatsTask(group);
		}
		else
		{
			//return new SpecialTask(group);
			assert(false);
		}
	}
	else if ( pat.is(SPREAD) )
	{
		return new SpreadingTask(group);
	}
	else if ( pat.is(RADIAL) )
	{
		return new RadiatingTask(group);
	}
	else if ( pat.is(FOCAL+ZONAL) )
	{
		return new FocalZonalTask(group);
	}
	else if ( pat.is(ZONAL) )
	{
		return new ZonalTask(group);
	}
	else if ( pat.is(FOCAL) )
	{
		return new FocalTask(group);
	}
	else if ( group->numdim() == D0)
	{
		return new ScalarTask(group);
	}
	else if ( pat.is(LOCAL) || pat.is(FREE) || pat.is(BARRIER))
	{
		return new LocalTask(group);
	}
	else {
		assert(0);
	}
}

Task::Task(Group *group)
	: base_group(nullptr)
	, prev_list()
	, next_list()
	, ver_list()
	, dep_hash()
	, prev_jobs_count(0)
	, self_jobs_count(0)
	, last()
	, mtx()
{
	// Links 'group' <-> 'task'
	group->task = this;
	this->base_group = group;

	// Links 'task' to its prev-tasks (which are guaranteed to have been generated previously)
	for (auto prev_group : group->prevList()) {
		Task *prev_task = prev_group->task;
		this->prev_list.push_back(prev_task);
		prev_task->next_list.push_back(this);
	}
	
	// Number of previous jobs that will notify this task (1 notif x job x output block)
	// @ revise this, draw some examples (what about spreading?)
	for (auto prev_task : prevList()) {
		for (auto node : inner_join(inputList(),prev_task->outputList())) {
			if (node->numdim() == D0) {
				prev_jobs_count += 1;
			} else {
				prev_jobs_count += prod(prev_task->numblock());
			}
		}
	}

	// Number of self jobs that will be issued by this task
	self_jobs_count = prod(numblock());

	// Filling next_of_out structure of prev_tasks
	for (auto prev_task : prevList())
		for (int i=0; i<prev_task->outputList().size(); i++)
			if (is_included(prev_task->outputList()[i],inputList()))
				prev_task->next_of_out[i].push_back(this);

	// Prepares next_of_out structure for the next tasks
	next_of_out.resize(outputList().size());

	// Filling is_input_of structure
	is_input_of.resize(inputList().size());
	for (int i=0; i<inputList().size(); i++)
		is_input_of[i] = isInputOf(inputList()[i],base_group);
}

int Task::id() const {
	return group()->id;
}

const Group* Task::group() const {
	return base_group;
}

const NodeList& Task::nodeList() const {
	return group()->nodeList();
}

const NodeList& Task::inputList() const {
	return group()->inputList();
}

const NodeList& Task::outputList() const {
	return group()->outputList();
}

const TaskList& Task::prevList() const {
	return prev_list;
}

const TaskList& Task::nextList() const {
	return next_list;
}

bool Task::isPrev(const Task *task) const {
	return this->group()->isPrev(task->group());
}

bool Task::isNext(const Task *task) const {
	return task->isPrev(this);
}

NumDim Task::numdim() const {
	return group()->numdim();
}

const DataSize& Task::datasize() const {
	return group()->datasize();
}

const BlockSize& Task::blocksize() const {
	return group()->blocksize();
}

const NumBlock& Task::numblock() const {
	return group()->numblock();
}

const VersionList& Task::versionList() const {
	return ver_list;
}

const Version* Task::version(DeviceType dev_type, std::string detail) const {
	for (auto &ver : ver_list) {
		bool typ_cond = (ver->deviceType() == dev_type || dev_type == DEV_ALL);
		bool det_cond = (ver->detail.compare(detail) == 0 || detail.empty());
		if (typ_cond && det_cond)
			return ver;
	}
	assert(!"No version matched");
}

void Task::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	in_keys.clear();

	for (auto node : inputList()) {
		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		in_keys.push_back( std::make_tuple(Key(node,coord),hold) );
	}
}

void Task::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	out_keys.clear();

	for (auto node : outputList()) {
		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		int depend = nextDependencies(node,coord);
		out_keys.push_back( std::make_tuple(Key(node,coord),hold,depend) );
	}
}

void Task::initialJobs(std::vector<Job> &job_vec) {
	Coord coord = {0,0};
	while (all(coord < numblock())) {
		job_vec.push_back( Job(this,coord) );
		coord = next(coord,numblock());
	}
}

void Task::askJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);	
	bool zero = (Tid == last);

	// Asks itself for self-jobs, a.k.a. intra-dependencies (e.g. Spreading, Radiating)
	this->selfJobs(done_job,job_vec);

	// Asks next-tasks for their next-jobs, a.k.a inter-dependencies (all Op)
	for (auto next_task : this->nextList()) {
		auto common_nodes = inner_join(this->outputList(),next_task->inputList());
		for (auto node : common_nodes) {
			if (node->numdim() == D0 && !zero)
				continue; // D0 jobs only notify once
			Key key = Key(node,done_job.coord);
			next_task->nextJobs(key,job_vec);
		}
	}
}

void Task::notify(Coord coord, std::vector<Job> &job_vec) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe

	auto it = dep_hash.find(coord);
	if (it == dep_hash.end()) { // Inserts an entry with the number of dependencies if one was not found
		int dep = selfDependencies(coord);
		auto pair = std::make_pair(coord,dep);
		it = dep_hash.insert(pair).first;
	}

	it->second--; // Reduces dependencies by 1
	assert(it->second >= 0);
	
	if (it->second == 0) { // Are all dependencies met?
		dep_hash.erase(it);
		job_vec.push_back( Job(this,coord) );
	}
}

void Task::notifyAll(std::vector<Job> &job_vec) {
	Coord coord = {0,0};
	while (all(coord < numblock())) {
		notify(coord,job_vec);
		coord = next(coord,numblock());
	}
}

int Task::selfDependencies(Coord coord) const {
	int dep = 0;
	for (auto node : inputList())
		dep += selfInterDepends(node,coord);
	for (auto node : outputList())
		dep += selfIntraDepends(node,coord);
	return dep;
}

int Task::nextDependencies(Node *node, Coord coord) const {
	int pos = list_position(node,outputList());
	int dep = 0;
	for (auto next_task : next_of_out[pos])
		dep += next_task->nextInterDepends(node,coord);
	dep += nextIntraDepends(node,coord);
	return dep;
}

void Task::preLoad(Coord coord) {
	return; // Nothing to do
}

void Task::preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	return; // Nothing to do
}

void Task::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	return; // Nothing to do
}

void Task::postStore(Coord coord) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe

	self_jobs_count--;
	assert(self_jobs_count >= 0);
	
	if (self_jobs_count == 0)
		last = Tid;
}

void Task::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	computeVersion(coord,in_blk,out_blk,ver);
}

void Task::computeVersion(Coord coord, const BlockList &in_blk, const BlockList &out_blk, const Version *ver) {
	// CL related vars
	cle::Task tsk = ver->tsk;
	cle::Kernel krn = tsk.K(Tid.rnk());
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
	const Config &conf = Runtime::getConfig();
	cl_int err;

	//// Configures kernel

	const int dim = numdim().toInt();
	auto group_size = ver->groupsize();
	auto block_size = blocksize();

	auto nsb = ((block_size-1)/group_size+1)*group_size;
	size_t gws[dim] = {(size_t)nsb[0],(size_t)nsb[1]};
	size_t lws[dim] = {(size_t)group_size[0],(size_t)group_size[1]};

	//// Sets kernel arguments

	int arg = 0, sidx = 0;

	for (auto &b : in_blk) {
		void *dev_mem = (b->entry != nullptr) ? b->entry->dev_mem : nullptr;
		/****/ if (b->holdtype() == HOLD_0) { // If HOLD_0, a null argument is given to the kernel
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &dev_mem);
			clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.get());
			clSetKernelArg(*krn, arg++, sizeof(b->fixed), &b->fixed);
		} else if (b->holdtype() == HOLD_1) { // If HOLD_1, a scalar argument is given
			clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.get());
		} else if (b->holdtype() == HOLD_N) { // In the normal case a valid cl_mem with memory is given
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &dev_mem);
			clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.get());
			clSetKernelArg(*krn, arg++, sizeof(b->fixed), &b->fixed);
		} else {
			assert(0);
		}
	}
	for (auto &b : out_blk) {
		/****/ if (b->holdtype() == HOLD_1) { // If HOLD_1, the scalar_page + index are given
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &b->scalar_page);
			int index = sizeof(double)*(conf.max_out_block*Tid.rnk() + sidx++);
			clSetKernelArg(*krn, arg++, sizeof(int), &index);
		} else if (b->holdtype() == HOLD_N) { // In the normal case a valid cl_mem with memory is given
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &b->entry->dev_mem);
		} else {
			assert(0);
		}
	}
	for (int i=0; i<dim; i++)
		clSetKernelArg(*krn, arg++, sizeof(int), &block_size[i]);
	for (int i=0; i<dim; i++)
		clSetKernelArg(*krn, arg++, sizeof(int), &coord[i]);
	for (int i=0; i<dim; i++)
		clSetKernelArg(*krn, arg++, sizeof(int), &group_size[i]);
	for (auto extra : ver->extra_arg)
		clSetKernelArg(*krn, arg++, sizeof(int), &extra);

	//// Launches kernel

	Runtime::getClock().start(KERNEL);

	err = clEnqueueNDRangeKernel(*que, *krn, dim, NULL, gws, lws, 0, nullptr, nullptr);
	err = clFinish(*que);
	cle::clCheckError(err);

	Runtime::getClock().stop(KERNEL);
}

} } // namespace map::detail
