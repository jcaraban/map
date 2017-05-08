/**
 * @file    Task.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Task.hpp"
#include "ScalarTask.hpp"
#include "RadialTask.hpp"
#include "SpreadTask.hpp"
#include "LoopTask.hpp"
#include "IdentityTask.hpp"
#include "../dag/Group.hpp"
#include "../ThreadId.hpp"
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
		return new SpreadTask(prog,clock,conf,group);
	}
	else if ( pat.is(RADIAL) )
	{
		return new RadialTask(prog,clock,conf,group);
	}
	else if ( group->numdim() == D0)
	{
		return new ScalarTask(prog,clock,conf,group);
	}
	else if ( pat.is(HEAD) || pat.is(TAIL)) {
		return new IdentityTask(prog,clock,conf,group);
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
	, prev_jobs_count(0)
	, self_jobs_count(0)
	, last()
	, accu_reach_of()
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

	// Filling is_input_of structure // @@ obsolete
	is_input_of.resize(inputList().size());
	for (int i=0; i<inputList().size(); i++)
		is_input_of[i] = isInputOf(inputList()[i],base_group);

	// Puts in + body + out nodes into 'all_list', in order
	auto all_list = full_join(inputList(),nodeList());
	all_list = full_unique_join(all_list,outputList());
	accu_reach_of = decltype(accu_reach_of)();

	// Walks nodes backward and accumulate their 'input spatial reach'
	for (auto i=all_list.rbegin(); i!=all_list.rend(); i++) {
		Node *node = *i;
		auto reach = Mask(numdim().unitVec(),true); // accumulated spatial reach

		auto next_inside = inner_join(node->nextList(),nodeList());
		for (auto next : next_inside) {
			auto next_in = next->inputReach(Coord());
			auto next_accu = accu_reach_of.find(next)->second;
			reach = flat(reach,pipe(next_in,next_accu)); // combines the spatial reaches
		}

		accu_reach_of.insert({node,reach});
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

Pattern Task::pattern() const {
	return group()->pattern();
}

const Mask& Task::inputReach(Node *node, Coord coord) const {
	auto it = accu_reach_of.find(node);
	assert(it != accu_reach_of.end());
	return it->second;
}

const Mask& Task::outputReach(Node *node, Coord coord) const {
	return node->outputReach(coord); // 'out_spa_rea' does not accumuate
}

void Task::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();

	// Generates a short list of most promising versions first
	VerkeyList key_list;
	for (int i=0; i<env.deviceSize(); i++) {
		Verkey key(this);
		key.dev = env.D(i);
		key.group = {16,16};
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

void Task::blocksToLoad(Coord coord, KeyList &in_keys) const {
	in_keys.clear();
	
	for (auto node : inputList()) {
		auto reach = inputReach(node,coord);
		auto space = reach.blockSpace(blocksize());

		for (auto offset : space) {	
			Coord nbc = coord + offset;
			HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
			hold = any(not in_range(nbc,numblock())) ? HOLD_0 : hold;
			Depend dep = node->isInput() ? nextInputDepends(node,nbc) : -1;

			in_keys.push_back( std::make_tuple(Key(node,nbc),hold,dep) );
		}
	}
}

void Task::blocksToStore(Coord coord, KeyList &out_keys) const {
	out_keys.clear();

	// TODO: add 'for offset : out_reach)

	for (auto node : outputList()) {
		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		int dep = nextDependencies(node,coord) + 1; // @ +1 for notify() in out-blks
		out_keys.push_back( std::make_tuple(Key(node,coord),hold,dep) );
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
	bool zero = (Tid == last);

	// Asks itself for self-jobs, a.k.a. intra-dependencies (e.g. Radial, Spread)
	this->selfJobs(done_job,job_vec);

	// Asks next-tasks for their next-jobs, a.k.a inter-dependencies (all Op)
	for (auto next_task : this->nextList()) {
		auto common_nodes = inner_join(this->outputList(),next_task->inputList());
		for (auto node : common_nodes) {
			if (node->numdim() == D0 && !zero)
				continue; // D0 jobs only notify at the end
			Key key = Key(node,done_job.coord);
			next_task->nextJobs(key,job_vec);
		}
	}
}

void Task::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void Task::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self!=D0
	{
		notifyAll(job_vec);
	}
	else // Case when prev!=D0, self!=D0
	{
		auto reach = inputReach(done_block.node,done_block.coord);
		auto space = reach.blockSpace(blocksize());

		for (auto offset : space) {	// @@ not really correct
			auto nbc = done_block.coord + offset;
			if (all(in_range(nbc,numblock()))) {
				notify(nbc,job_vec);
			}
		}
	}
}

void Task::notify(Coord coord, std::vector<Job> &job_vec) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe

	auto it = dep_hash.find(coord);
	if (it == dep_hash.end()) { // not found, inserts an entry with the number of dependencies if one was not found
		int dep = prevDependencies(coord);
		auto pair = std::make_pair(coord,dep);
		it = dep_hash.insert(pair).first;
	}

	// Notifies, i.e. reduces dependencies by 1
	it->second--;
	assert(it->second >= 0);
	
	 // Are all dependencies met?
	if (it->second == 0) {
		dep_hash.erase(it);
		job_vec.push_back( Job(this,coord) );
	}
}

void Task::notifyAll(std::vector<Job> &job_vec) {
	auto beg = Coord(numblock().size(),0);
	auto end = numblock();
	for (auto coord : iterSpace(beg,end)) {
		notify(coord,job_vec);
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
	auto reach = inputReach(node,coord);
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
			auto reach = inputReach(node,coord);
			auto space = reach.blockSpace(blocksize());
			for (auto offset : space)
				dep += all(in_range(coord+offset,numblock()));
		}
	}
	
	return dep;
}

void Task::preLoad(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	if (not Runtime::getConfig().prediction)
		return;
	if (numdim() == D0)
		return; // for D0, fixing is more costly than just computing D0 values

	std::unordered_map<Key,ValFix,key_hash> val_hash; // Supporting hash for the fixed values

	// Fills inputs first with 'in_blk'
	for (auto in : in_blk) {
		if (in->holdtype() == HOLD_0) // When the block is null, looks for the central block
		{
			auto pred = [&](const Block *b){ return b->key == Key{in->key.node,coord}; };
			auto it = std::find_if(in_blk.begin(),in_blk.end(),pred);
			assert(it != in_blk.end());

			val_hash[in->key] = ValFix((*it)->value,(*it)->fixed);
		}
		else // HOLD_1 or HOLD_N
		{
			val_hash[in->key] = ValFix(in->value,in->fixed);
		}
	}

	// Iterates the nodes to fill 'value_list' and 'fixed_list'
	NodeList nodes_to_fill = full_unique_join(nodeList(),outputList());

	for (auto node : nodes_to_fill) {
		auto reach = inputReach(node,coord);
		auto space = reach.blockSpace(blocksize());
		for (auto offset : space) {
			node->computeFixed(coord+offset,val_hash);
		}
	}

	// Transfer outputs to 'out_blk'
	for (auto out : out_blk) {
		assert(val_hash.find(out->key) != val_hash.end());
		out->fixValue( val_hash[out->key] );
	}
}

void Task::preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
	cl_int err;

	// Fills output-zonal-scalars with neutral value
	int sidx = 0;
	for (auto blk : out_blk) {
		if (blk->holdtype() == HOLD_1)
		{
			ReductionType rtype;
			auto *zn = dynamic_cast<ZonalReduc*>(blk->key.node);
			if(zn != nullptr)
				rtype = zn->type;
			auto *sn = dynamic_cast<Scalar*>(blk->key.node);
			if(sn != nullptr) {
				auto *zn = dynamic_cast<ZonalReduc*>(sn->prev());
				assert(zn != nullptr);
				rtype = zn->type;
			}
			assert(rtype != NONE_REDUCTION);

			int index = sizeof(double)*(conf.max_out_block*Tid.rnk() + sidx++);
			blk->value = rtype.neutral(blk->datatype());
			size_t dtsz = blk->value.datatype().sizeOf();

			err = clEnqueueFillBuffer(*que,blk->scalar_page,&blk->value.ref(),dtsz,index,dtsz,0,nullptr,nullptr);
			cle::clCheckError(err);
		}
	}
}

void Task::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());

	// Transfers back the output-zonal-scalars
	int sidx = 0;
	for (auto blk : out_blk) {
		if (blk->holdtype() == HOLD_1)
		{
			int index = sizeof(double)*(conf.max_out_block*Tid.rnk() + sidx++);
			size_t dtsz = blk->value.datatype().sizeOf();

			cl_int clerr = clEnqueueReadBuffer(*que,blk->scalar_page,CL_TRUE,index,dtsz,&blk->value.ref(),0,nullptr,nullptr);
			cle::clCheckError(clerr);

			if (blk->key.node->pattern().is(ZONAL)) { // @
				auto *node = dynamic_cast<ZonalReduc*>(blk->key.node);
				std::lock_guard<std::mutex> lock(mtx); // thread-safe
				node->value = node->type.apply(node->value,blk->value);
			}
		}

		//... continue ... cl Enqueue Fill/Read Buffer with the 'group_page' too, then upgrade the 'skeletons'

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
			blk->key.node->stats.minb[idx] = min.get();
			blk->key.node->stats.maxb[idx] = max.get();
			blk->key.node->stats.meanb[idx] = mean.get();
			blk->key.node->stats.stdb[idx] = std.get();

			// If the block values are the same, fix the value
			if (max == min) {
				blk->fixValue( ValFix(max,true) );
			}
		}
	}
}

void Task::postStore(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe

	self_jobs_count--;
	assert(self_jobs_count >= 0);
	
	if (self_jobs_count == 0)
		last = Tid;
}

void Task::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail

	auto all_fixed = [&](Block *b){ return b->fixed; };
	if (std::all_of(out_blk.begin(),out_blk.end(),all_fixed))
		return; // All output blocks are fixed, no need to compute

	computeVersion(coord,in_blk,out_blk,ver);
}

void Task::computeVersion(Coord coord, const BlockList &in_blk, const BlockList &out_blk, const Version *ver) {
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

	auto nsb = ((block_size-1)/group_size+1)*group_size;
	size_t gws[dim] = {(size_t)nsb[0],(size_t)nsb[1]};
	size_t lws[dim] = {(size_t)group_size[0],(size_t)group_size[1]};

	//// Sets kernel arguments

	int arg = 0, sidx = 0;

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

	clock.start(KERNEL);

	err = clEnqueueNDRangeKernel(*que, *krn, dim, NULL, gws, lws, 0, nullptr, nullptr);
	err = clFinish(*que);
	cle::clCheckError(err);

	clock.stop(KERNEL);
}

} } // namespace map::detail
