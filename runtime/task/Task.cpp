/**
 * @file    Task.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: SPREAD needs another look in nextJobs(), 'for offset : out_space'
 * TODO: nextJob() and the 'inversion' would not work for the central Radial Job
 */

#include "Task.hpp"
#include "ScalarTask.hpp"
#include "RadialTask.hpp"
#include "LoopTask.hpp"
#include "TailTask.hpp"
#include "IdentityTask.hpp"
#include "../dag/Group.hpp"
#include "../Runtime.hpp"
#include <memory>
#include <cassert>


namespace map { namespace detail {

Task* Task::Factory(Program &prog, Clock &clock, Config &conf, Group *group) {
	Pattern pat = group->pattern();

	/**/ if ( pat.is(LOOP) )
	{
		return new LoopTask(prog,clock,conf,group);
	}
	else if ( pat.is(SPREAD) )
	{
		assert(0); //return new SpreadTask(prog,clock,conf,group);
	}
	else if ( pat.is(RADIAL) )
	{
		return new RadialTask(prog,clock,conf,group);
	}
	else if ( group->numdim() == D0)
	{
		return new ScalarTask(prog,clock,conf,group);
	}
	else if ( pat.is(TAIL)) {
		return new TailTask(prog,clock,conf,group);
	}
	else {
		return new Task(prog,clock,conf,group);
	}
}

Task::Task(Program &prog, Clock &clock, Config &conf, Group *group)
	: prog(prog)
	, clock(clock)
	, conf(conf)
	, base_group(nullptr)
	, prev_list()
	, next_list()
	, ver_list()
	, dep_hash()
	, prev_jobs_count()
	, self_jobs_count()
	, last()
	, accu_in_reach_of()
	, accu_out_reach_of()
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

	// Links 'task' to its back-tasks, and makes the back-tasks link forward to 'this'
	for (auto back_group : group->backList()) {
		Task *back_task = back_group->task;
		this->back_list.push_back(back_task);
		back_task->forw_list.push_back(this);
	}
	
	// Number of previous jobs that will notify this task
	for (auto prev_task : prevList()) {
		for (auto node : inner_join(inputList(),prev_task->outputList())) {
			if (node->numdim() == D0) {
				prev_jobs_count[0] += 1;
			} else {
				prev_jobs_count[0] += prod(prev_task->numblock());
			}
		}
	}

	// Number of self jobs that will be issued by this task
	self_jobs_count[0] = prod(numblock());

	// Filling 'next_of_out' structure with 'prev_tasks'
	for (auto prev_task : prevList())
		for (int i=0; i<prev_task->outputList().size(); i++)
			if (is_included(prev_task->outputList()[i],this->inputList()))
				prev_task->next_of_out[i].push_back(this);

	// Prepares next_of_out structure for the next tasks
	next_of_out.resize(outputList().size());

	// Filling 'next_of_out' structure with 'back_tasks'
	for (auto back_task : backList())
		for (int i=0; i<this->outputList().size(); i++)
			if (is_included(this->outputList()[i],back_task->inputList()))
				this->next_of_out[i].push_back(back_task);

	// Filling 'is_input_of' structure, e.g. tells the pre-focal, pre-radial
	is_input_of.resize(inputList().size());
	for (int i=0; i<inputList().size(); i++)
		is_input_of[i] = isInputOf(inputList()[i],base_group);

	// Puts in + body + out nodes into 'all_list', in order
	auto body_out = full_unique_join(nodeList(),outputList());
	auto all_list = full_join(inputList(),body_out);

	// Walks nodes backward and accumulate their 'input spatial reach'
	for (auto i=all_list.rbegin(); i!=all_list.rend(); i++) {
		Node *node = *i;
		auto reach = Mask(numdim().unitVec(),true); // accumulated spatial reach

		auto next_inside = inner_join(node->nextList(),nodeList());
		for (auto next : next_inside) {
			auto next_in = next->inputReach();
			auto next_accu = accu_in_reach_of.find(next)->second;
			reach = flat(reach,pipe(next_in,next_accu)); // combines the spatial reaches
		}

		accu_in_reach_of.insert({node,reach});
	}

	// Walks nodes forward and accumulates their 'output spatial reach'
	for (auto node : body_out) { // @ all_list ?
		auto reach = Mask(numdim().unitVec(),true); // accumulated spatial reach

		auto prev_inside = inner_join(node->prevList(),nodeList());
		for (auto prev : prev_inside) {
			auto prev_out = prev->outputReach();
			auto prev_accu = accu_out_reach_of.find(prev)->second;
			reach = flat(reach,pipe(prev_out,prev_accu)); // combines the spatial reaches
		}

		accu_out_reach_of.insert({node,reach});
	}
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

const TaskList& Task::backList() const {
	return back_list;
}

const TaskList& Task::forwList() const {
	return forw_list;
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

const GroupSize& Task::groupsize() const {
	return group()->groupsize();
}

const NumGroup& Task::numgroup() const {
	return group()->numgroup();
}

Pattern Task::pattern() const {
	return group()->pattern();
}

const Mask& Task::accuInputReach(Node *node, Coord coord) const {
	auto it = accu_in_reach_of.find(node);
	assert(it != accu_in_reach_of.end());
	return it->second;
}

const Mask& Task::accuOutputReach(Node *node, Coord coord) const {
	auto it = accu_out_reach_of.find(node);
	assert(it != accu_out_reach_of.end());
	return it->second;
}

void Task::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();

	// Generates a short list of most promising versions first
	VerkeyList key_list;
	for (int i=0; i<env.deviceSize(); i++) {
		Verkey key(this);
		key.dev = env.D(i);
		key.group = groupsize();
		key.detail = "";
		key_list.push_back(key);
	}

	// Create the versions if they did not exist yet
	for (auto key : key_list) {
		const Version *ver = getVersion(cledev2devtype(key.dev),key.group,key.detail);
		if (ver == nullptr) {
			Version *ver = new Version(key);
			Runtime::getInstance().addVersion(ver); // Adds version to Runtime
			ver_list.push_back(ver);  // Adds version to Task
		}
	}
}

const VersionList& Task::versionList() const {
	return ver_list;
}

const Version* Task::getVersion(DeviceType dev_type, GroupSize group_size, std::string detail) const {
	for (auto &ver : ver_list) {
		bool devtype_cond = dev_type == DEV_ALL || ver->deviceType() == dev_type;
		bool group_cond = group_size.isNone() || all(ver->group_size == group_size);
		bool detail_cond = detail.empty() || ver->detail.compare(detail) == 0;
		if (devtype_cond && group_cond && detail_cond)
			return ver;
	}
	return nullptr;
}

void Task::blocksToLoad(Job job, KeyList &in_key) const {
	in_key.clear();
	
	for (auto node : inputList()) {
		auto reach = accuInputReach(node,job.coord);
		auto space = reach.blockSpace(blocksize());

		for (auto offset : space) {	
			Coord nbc = job.coord + offset;
			HoldType hold = node->holdtype(nbc);
			Depend dep = node->isInput() ? nextInputDepends(node,nbc) : -1;

			in_key.push_back( std::make_tuple(Key(node,nbc,job.iter),hold,dep) );
		}
	}
}

void Task::blocksToStore(Job job, KeyList &out_key) const {
	out_key.clear();

	for (auto node : outputList()) {
		auto reach = accuOutputReach(node,job.coord);
		auto space = reach.blockSpace(blocksize());

		for (auto offset : space) {	
			Coord nbc = job.coord + offset;
			HoldType hold = node->holdtype(nbc);
			int dep = 1 + nextDependencies(node,nbc); // +1 cause out blocks get 1 extra notify()
			out_key.push_back( std::make_tuple(Key(node,nbc,job.iter),hold,dep) );
		}
	}
}

void Task::initialJobs(std::vector<Job> &job_vec) {
	auto beg = Coord(numblock().size(),0);
	auto end = numblock();
	for (auto coord : iterSpace(beg,end)) {
		job_vec.push_back( Job(this,coord) );
	}
}

void Task::askJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);

	// Asks itself for self-jobs, a.k.a. intra-dependencies (e.g. Radial, Spread)
	this->selfJobs(done_job,job_vec);

	// Asks next-tasks for their next-jobs, a.k.a inter-dependencies (all Op)
	for (auto next_task : full_join(nextList(),backList())) {
		next_task->nextJobs(done_job,job_vec,Tid==last);
	}

	if (Tid == last) {
		std::lock_guard<std::mutex> lock(mtx);
		last = ThreadId(); // @
	}
}

void Task::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void Task::nextJobs(Job done_job, std::vector<Job> &job_vec, bool end) {
	auto iter = done_job.iter;
	auto prev_nodes = done_job.task->outputList();
	auto common_nodes = inner_join(inputList(),prev_nodes);

	for (auto node : common_nodes) {
		if (node->isReduction()) {
			if (not end) { // D0 jobs only notify at the end
				continue;
			} else { // Case when prev=D0, self!=D0
				notifyAll( Job(this,Coord(),iter), job_vec);
			}
		} else if (node->numdim() == D0 && done_job.task->numdim() == D0 && this->numdim() == D2) {
			if (not end) {
				continue;
			} else {
				notifyAll( Job(this,Coord(),iter), job_vec);
			}
		} else { // Case when prev!=D0, self!=D0
			auto reach = accuInputReach(node,done_job.coord);
			auto inver = reach.invert(); // Notifies the inverted 'input space'
			auto space = inver.blockSpace(blocksize());

			for (auto offset : space) {
				auto nbc = done_job.coord + offset;
				if (all(in_range(nbc,numblock()))) {
					Job new_job = Job(this,nbc,iter);
					notify(new_job,job_vec);
				}
			}
		}
	}
}

void Task::notify(Job new_job, std::vector<Job> &job_vec) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe
	auto coord = new_job.coord;
	auto iter = new_job.iter;

	auto it = dep_hash.find(coord);
	if (it == dep_hash.end()) { // not found, inserts an entry with the number of dependencies if one was not found
		int dep = prevDependencies(coord);
		auto pair = std::make_pair(coord,dep);
		it = dep_hash.insert(pair).first;
	}

//std::cout << " " << new_job.task->id() << " " << new_job.coord << " " << new_job.iter << " : " << it->second << std::endl;
	// Notifies, i.e. reduces dependencies by 1
	it->second--;
	assert(it->second >= 0);
	
	 // Are all dependencies met?
	if (it->second == 0) {
		dep_hash.erase(it);
		job_vec.push_back(new_job);

		if (self_jobs_count.find(iter) == self_jobs_count.end())
			self_jobs_count[iter] = prod(numblock()); // @@
	}
}

void Task::notifyAll(Job new_job, std::vector<Job> &job_vec) {
	auto beg = Coord(numblock().size(),0);
	auto end = numblock();
	for (auto coord : iterSpace(beg,end)) {
		notify( Job(this,coord,new_job.iter), job_vec);
	}
}

int Task::prevDependencies(Coord coord) const {
	int dep = 0;
	for (auto node : inputList())
		dep += prevInterDepends(node,coord);
	for (auto node : outputList())
		dep += prevIntraDepends(node,coord);
	return dep;
}

int Task::nextDependencies(Node *node, Coord coord) const {
	int pos = value_position(node,outputList());
	int dep = 0;
	for (auto next_task : next_of_out[pos])
		dep += next_task->nextInterDepends(node,coord);
	dep += nextIntraDepends(node,coord);
	return dep;
}

int Task::prevInterDepends(Node *node, Coord coord) const {
	auto reach = accuInputReach(node,coord);
	auto space = reach.blockSpace(blocksize());
	int dep = 0;

	for (auto offset : space) {
		Coord nbc = coord + offset;
		if (all(in_range(nbc,numblock()))) {
			dep += (node->pattern()==INPUT || node->pattern()==FREE) ? 0 : 1;
		}
	}
	
	return dep;
}

int Task::nextInterDepends(Node *node, Coord coord) const {
	return prevInterDepends(node,coord); // prevInter != nextInter in Radial
}

int Task::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Local / Focal / Zonal do not present intra dependencies
}

int Task::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Local / Focal / Zonal do not present intra dependencies
}

int Task::nextInputDepends(Node *node, Coord coord) const { // @
	assert(node->isInput());
	int dep = 0;

	for (auto task : prog.taskList()) {
		if (is_included(node,task->inputList())) {
			auto reach = accuInputReach(node,coord);
			auto space = reach.blockSpace(blocksize());
			for (auto offset : space)
				dep += all(in_range(coord+offset,numblock()));
		}
	}
	
	return dep;
}

void Task::preLoad(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	if (not Runtime::getConfig().prediction)
		return;
	if (numdim() == D0)
		return; // for D0, fixing is more costly than just computing D0 values

	std::unordered_map<Key,ValFix,key_hash> val_hash; // Supporting hash for the fixed values
	auto coord = job.coord;

	// Fills inputs first with 'in_blk'
	for (auto in : in_blk) {
		if (in->holdtype() == HOLD_0) // When the block is null, looks for the central block
		{
			Key in_key = Key(in->key.node,job.coord,job.iter);
			auto pred = [&](const Block *b){ return b->key == in_key; };
			auto it = std::find_if(in_blk.begin(),in_blk.end(),pred);
			assert(it != in_blk.end());

			in_key.iter = 0;
			val_hash[in_key] = ValFix((*it)->value,(*it)->fixed);
		}
		else // HOLD_1 or HOLD_N
		{
			Key in_key = Key(in->key.node,job.coord);
			val_hash[in_key] = ValFix(in->value,in->fixed);
		}
	}

	// Iterates the nodes to fill 'value_list' and 'fixed_list'
	NodeList nodes_to_fill = full_unique_join(nodeList(),outputList());

	for (auto node : nodes_to_fill) {
		auto reach = accuInputReach(node,coord);
		auto space = reach.blockSpace(blocksize());
		for (auto offset : space) {
			node->computeFixed(coord+offset,val_hash);
		}
	}

	// Transfer outputs to 'out_blk'
	for (auto out : out_blk) {
		Key out_key = Key(out->key.node,job.coord);
		assert(val_hash.find(out_key) != val_hash.end());
		out->fixValue( val_hash[out_key] );
	}
}

void Task::preCompute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	return; // choose a code version, according to availabe statistics ?
}

void Task::postCompute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	return; // collect and update statistics ?
}

void Task::postStore(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	auto coord = job.coord;

	// @ Integrates 'stats' to the block and node
	for (auto blk : out_blk) {
		if (blk->key.node->pattern().isNot(STATS))
			continue;
		auto *summary = dynamic_cast<Summary*>(blk->key.node);
		if (summary != nullptr) {
			VariantType min, max, mean, std;

			// Finds the blocks storing the individual statistics
			for (auto b : out_blk) {
				if (b->key.node == summary->min())
					min = b->value;
				if (b->key.node == summary->max())
					max = b->value;
				if (b->key.node == summary->mean())
					mean = b->value;
				if (b->key.node == summary->std())
					std = b->value;
			}

			// Fills the 'block' with the statistics
			blk->stats.active = true;
			blk->stats.min = min.get();
			blk->stats.max = max.get();
			blk->stats.mean = mean.get();
			blk->stats.std = std.get();

			// Fills the 'node' with the statistics
			int idx = proj(coord,blk->key.node->numblock());
			blk->key.node->stats.minb[idx]  = min.get();
			blk->key.node->stats.maxb[idx]  = max.get();
			blk->key.node->stats.meanb[idx] = mean.get();
			blk->key.node->stats.stdb[idx]  = std.get();

			// If the block values are the same, fix the value
			if (max == min) {
				blk->fixValue( ValFix(max,true) );
			}
		}
	}
}

void Task::postWork(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe
	auto iter = job.iter;
	assert(self_jobs_count[iter] > 0);

	self_jobs_count[iter]--;
	if (self_jobs_count[iter] == 0) {
		last = Tid;
		self_jobs_count.erase(iter);
	}

	// @ Integrates reduced zonal value to the node
	if (last == Tid) {
		for (auto blk : out_blk) {
			blk->key.node->value = blk->value;
		}
	}
}

void Task::compute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	assert(ver != nullptr);

	auto all_fixed = [&](Block *b){ return b->fixed; };
	if (std::all_of(out_blk.begin(),out_blk.end(),all_fixed)) {
		clock.incr(NOT_COMPUTED);
		return; // All output blocks are fixed, no need to compute
	}

	computeVersion(job,in_blk,out_blk,ver);
}

void Task::computeVersion(Job job, const BlockList &in_blk, const BlockList &out_blk, const Version *ver) {
	clock.incr(COMPUTED);

	// CL related vars
	cle::Task tsk = ver->tsk;
	cle::Kernel krn = tsk.K(Tid.rnk());
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
	cl_int err;

	//// Configures kernel

	assert(numdim().toInt() == 2);
	const int dim = 2;
	auto group_size = ver->groupsize();
	auto block_size = blocksize();
	auto coord = job.coord;

	auto nsb = ((block_size-1)/group_size+1)*group_size;
	size_t gws[dim] = {(size_t)nsb[0],(size_t)nsb[1]};
	size_t lws[dim] = {(size_t)group_size[0],(size_t)group_size[1]};

	//// Sets kernel arguments

	int arg = 0;

	for (auto &b : in_blk) {
		void *dev_mem = (b->entry != nullptr) ? b->entry->dev_mem : nullptr;
		/****/ if (b->holdtype() == HOLD_0) { // If HOLD_0, a null argument is given to the kernel
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &dev_mem);
			clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.ref());
			clSetKernelArg(*krn, arg++, sizeof(b->fixed), &b->fixed);
		} else if (b->holdtype() == HOLD_1) { // If HOLD_1, a scalar argument is given
			clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.ref());
		} else if (b->holdtype() == HOLD_N) { // In the normal case a valid cl_mem with memory is given
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &dev_mem);
			clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.ref());
			clSetKernelArg(*krn, arg++, sizeof(b->fixed), &b->fixed);
		} else {
			assert(0);
		}
	}
	for (auto &b : out_blk) {
		/****/ if (b->holdtype() == HOLD_1) { // If HOLD_1, the scalar_page + offset are given
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &b->scalar_page);
			int offset = sizeof(double)*(conf.max_out_block*Tid.rnk() + b->order);
			clSetKernelArg(*krn, arg++, sizeof(int), &offset);
		//} else if (b->holdtype() == HOLD_2) {
		//	clSetKernelArg(*krn, arg++, sizeof(cl_mem), &b->group_page);
		//	int offset = sizeof(double)*conf.max_group_x_block*(conf.max_out_block*Tid.rnk() + b->order);
		//	clSetKernelArg(*krn, arg++, sizeof(int), &offset);
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

	clock.start(KERNEL);

	err = clEnqueueNDRangeKernel(*que, *krn, dim, NULL, gws, lws, 0, nullptr, nullptr);
	err = clFinish(*que);
	cle::clCheckError(err);

	clock.stop(KERNEL);
}

} } // namespace map::detail
