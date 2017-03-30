/**
 * @file    SpreadingTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "SpreadingTask.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

/*************
   Spreading
 *************/

SpreadingTask::SpreadingTask(Group *group)
	: Task(group)
{
	// @ TODO: fix this somehow
	for (auto node : group->nodeList()) {
		if (node->pattern().is(SPREAD)) {
			auto *scan = dynamic_cast<SpreadScan*>(node);
			assert(scan != nullptr);
			this->scan = scan;
			break;
		}
	}

	createVersions();
}

void SpreadingTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void SpreadingTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	in_keys.clear();
	for (int i=0; i<inputList().size(); i++)
	{
		Node *node = inputList()[i];
		const int N = is_input_of[i].is(SPREAD) ? 1 : 0;
		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		
		for (int y=-N; y<=N; y++) {
			for (int x=-N; x<=N; x++) {
				auto nbc = coord + Coord{x,y};
				HoldType hold_nbc = (any(nbc < 0) || any(nbc >= numblock())) ? HOLD_0 : hold;
				in_keys.push_back( std::make_tuple(Key(node,nbc),hold_nbc) );
			}
		}
	}
}

void SpreadingTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	out_keys.clear();

	// All non-SPREAD outputs first
	for (int i=0; i<outputList().size(); i++)
	{
		Node *node = outputList()[i];

		if (node->pattern().is(SPREAD))
			continue; // No spread node now

		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		int depend = -1; // non-discardable because unknown dependencies and stability

		out_keys.push_back( std::make_tuple(Key(node,coord),hold,depend) );
	}

	// Requires its own output in previous blocks
	// Scan 3x3 output blocks
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			auto nbc = coord + Coord{x,y};
			HoldType hold = (any(nbc < 0) || any(nbc >= numblock())) ? HOLD_0 : HOLD_N;
			out_keys.push_back( std::make_tuple(Key(scan,nbc),hold,-1) );
		}
	}
	// Spread 3x3 output nodes
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			auto nbc = coord + Coord{x,y};
			HoldType hold = (any(nbc < 0) || any(nbc >= numblock())) ? HOLD_0 : HOLD_N;
			out_keys.push_back( std::make_tuple(Key(scan->spread(),nbc),hold,-1) );
		}
	}
	// Buffer output block
	out_keys.push_back( std::make_tuple(Key(scan->buffer(),coord),HOLD_N,-1) );
	// Stable output block
	out_keys.push_back( std::make_tuple(Key(scan->stable(),coord),HOLD_N,-1) );
}

void SpreadingTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);
}

void SpreadingTask::askJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);	

	// Ask next-tasks only in the last iteration, when the block is stable
	mtx.lock(); // thread-safe
	auto it = stable_hash.find(done_job.coord);
	assert(it != stable_hash.end());
	stable_vec vec = it->second;
	bool unstable = vec[1][1];
	mtx.unlock();

	if (unstable)
	{
		// Asks itself for self-jobs, a.k.a. intra-dependencies (e.g. Spreading, Radiating)
		this->selfJobs(done_job,job_vec);
	}
	else // stable
	{
		// Removes stable mark
		mtx.lock(); // thread-safe
		stable_hash.erase(done_job.coord);
		mtx.unlock();

		// Self jobs decremented only in the last iteration
		mtx.lock(); // thread-safe
		self_jobs_count--;
//std::cout << "  " << done_job.task->id() << done_job.coord << " " << self_jobs_count << std::endl;
		assert(self_jobs_count >= 0);
		bool zero = (self_jobs_count == 0);
		mtx.unlock();

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
}

void SpreadingTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);

	mtx.lock(); // thread-safe
	auto it = stable_hash.find(done_job.coord);
	assert(it != stable_hash.end());
	stable_vec vec = it->second;
	mtx.unlock();

	for (int y=-1; y<=1; y++)
		for (int x=-1; x<=1; x++)
			if (vec[y+1][x+1])
				notify(done_job.coord+Coord{x,y},job_vec);
}

void SpreadingTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
	{
		notifyAll(job_vec);
	}
	else // Case when prev=D2, self=D2
	{
		int pos = value_position(done_block.node,inputList());
		const int N = is_input_of[pos].is(SPREAD) ? 1 : 0;

		for (int y=-N; y<=N; y++) {
			for (int x=-N; x<=N; x++) {
				auto nbc = done_block.coord + Coord{x,y};
				if (all(nbc >= 0) && all(nbc < numblock())) {
					notify(nbc,job_vec);
				}
			}
		}
	}
}

int SpreadingTask::prevInterDepends(Node *node, Coord coord) const {
	if (first_time.find(coord) != first_time.end())
		return 0; // There are no inter-dependencies after the first initial job

	int pos = value_position(node,inputList());
	if (!is_input_of[pos].is(SPREAD))
		return node->pattern() == FREE ? 0 : 1;
	
	// Spread inputs depend on their neighborhood
	int depend = 0;
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			Coord nbc = coord + Coord{x,y};
			if (all(nbc >= 0) && all(nbc < numblock()))
				depend += node->isInput() ? 0 : 1;
		}
	}
	return depend;
}

int SpreadingTask::nextInterDepends(Node *node, Coord coord) const {
	return prevInterDepends(node,coord); // @ reusing prevInterDepends, but would need own code
}

int SpreadingTask::prevIntraDepends(Node *node, Coord coord) const {
	return (first_time.find(coord) == first_time.end()) ? 0 : 1;
	// After the first initial job, even 1 self-job should be able to activate this job
	// de cuantos self-blocks dependo yo?
}

int SpreadingTask::nextIntraDepends(Node *node, Coord coord) const {
	return -1; // Unknown
	// cuantos self-blocks dependen de mi?
}

void SpreadingTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = version(DEV_ALL,""); // Any device, no detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());

	// if 'first time' in this coordinate, fills the accumulation buffers
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			Coord nbc = coord + Coord{x,y};
			if (any(nbc < 0) || any(nbc >= numblock()))
				continue;
			if (first_time.find(nbc) == first_time.end()) {
				first_time.insert(nbc);
				fillScanBuffer(nbc,in_blk,out_blk,que);
				fillSpreadBuffer(nbc,in_blk,out_blk,que);
				//fillStableBuffer(nbc,in_blk,out_blk,que);
			}
		}
	}

	BlockList in_list(in_blk.begin(),in_blk.begin()+9); // @ only Dir blocks
	Task::computeVersion(coord,in_list,out_blk,ver);

	swapSpreadBuffer(coord,in_blk,out_blk,que);

	auto vec = readStableVec(coord,in_blk,out_blk,que);

	mtx.lock(); // thread-safe
	stable_hash[coord] = vec; // Updates the stable structure
	mtx.unlock();

	// If the out-block is stable, make sure to pass 'write=true' to release-Output-Block
}

void SpreadingTask::fillScanBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que) {
	assert(all(coord >= 0) && all(coord < numblock()));
	
	Block *blk = nullptr;
	VariantType neutral = scan->type.neutral(scan->datatype());

	for (auto &b : out_blk)
		blk = (b->key.node == scan && all(b->key.coord == coord)) ? b : blk;
	assert(blk != nullptr);

	cl_int err = clEnqueueFillBuffer(*que,blk->entry->dev_mem,&neutral.get(),neutral.datatype().sizeOf(),0,blk->total_size,0,nullptr,nullptr);
	cle::clCheckError(err);
}

void SpreadingTask::fillSpreadBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que) {
	assert(all(coord >= 0) && all(coord < numblock()));
	
	Block *dst = nullptr;
	for (auto &b : out_blk)
		dst = (b->key.node == scan->spread() && all(b->key.coord == coord)) ? b : dst;
	assert(dst != nullptr);

	if (scan->prev()->numdim() != D0)
	{
		Block *src = nullptr;
		for (auto &b : in_blk)
			src = (b->key.node == scan->prev() && all(b->key.coord == coord)) ? b : src;
		assert(src != nullptr);

		cl_int err = clEnqueueCopyBuffer(*que,src->entry->dev_mem,dst->entry->dev_mem,0,0,src->total_size,0,nullptr,nullptr);
		cle::clCheckError(err);		
	}
	else // D0 case
	{
		auto constant = dynamic_cast<Constant*>(scan->prev());
		assert(constant != nullptr); // @ Only constant is accepted as D0 for the moment
		VariantType cnst = constant->cnst;

		cl_int err = clEnqueueFillBuffer(*que,dst->entry->dev_mem,&cnst.get(),cnst.datatype().sizeOf(),0,dst->total_size,0,nullptr,nullptr);
		cle::clCheckError(err);
	}
}

void SpreadingTask::fillStableBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que) {
	assert(all(coord >= 0) && all(coord < numblock()));
	
	Block *blk = nullptr;
	VariantType neutral = ReductionType(rOR).neutral(U16);

	for (auto &b : out_blk)
		blk = (b->key.node == scan->stable() && all(b->key.coord == coord)) ? b : blk;
	assert(blk != nullptr);

	cl_int err = clEnqueueFillBuffer(*que,blk->entry->dev_mem,&neutral.get(),neutral.datatype().sizeOf(),0,blk->total_size,0,nullptr,nullptr);
	cle::clCheckError(err);
}

void SpreadingTask::swapSpreadBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que) {
	assert(all(coord >= 0) && all(coord < numblock()));
	
	Block *src = nullptr, *dst = nullptr;
	for (auto &b : out_blk)
		src = (b->key.node == scan->buffer() && all(b->key.coord == coord)) ? b : src;
	assert(src != nullptr);
	for (auto &b : out_blk)
		dst = (b->key.node == scan->spread() && all(b->key.coord == coord)) ? b : dst;
	assert(dst != nullptr);

	cl_int err = clEnqueueCopyBuffer(*que,src->entry->dev_mem,dst->entry->dev_mem,0,0,src->total_size,0,nullptr,nullptr);
	cle::clCheckError(err);
}

stable_vec SpreadingTask::readStableVec(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que) {
	assert(all(coord >= 0) && all(coord < numblock()));

	Block *blk = nullptr;
	for (auto &b : out_blk)
		blk = (b->key.node == scan->stable()) ? b : blk;
	assert(blk != nullptr);

	//@@ blk->map(IN); // Mapping memory, from Device to Host
	stable_vec vec;

	int num_elem = blk->total_size / (16*16) / blk->datatype().sizeOf();
	auto *src = static_cast<const Ctype<U16>*>(blk->entry->host_mem);

	Ctype<U16> tmp = NeutralOperator<rOR,U16>()();
	for (int i=0; i<num_elem; i++)
		tmp = ReductionOperator<rOR,U16>()(tmp,src[i]);

//std::cout << num_elem << ": " << id() << coord << " " << tmp << std::endl;

	vec[1][1] = tmp;
	vec[1][2] = tmp & 0x02;
	vec[2][2] = tmp & 0x04;
	vec[2][1] = tmp & 0x08;
	vec[2][0] = tmp & 0x10;
	vec[1][0] = tmp & 0x20;
	vec[0][0] = tmp & 0x40;
	vec[0][1] = tmp & 0x80;
	vec[0][2] = tmp & 0x100;

	// Coords outside the borders become 'false'
	for (int y=-1; y<=1; y++)
		for (int x=-1; x<=1; x++)
			if (any(coord+Coord{x,y} < 0) || any(coord+Coord{x,y} >= numblock()))
				vec[y+1][x+1] = false;
	
	//@@ blk->unmap(IN); // Unmapping memory, back from Host to Device
	return vec;
}

unsigned int SpreadingTask::height(Coord coord) const {
	unsigned int height = 0;

	if (scan->prev()->isInput()) {
		auto in = dynamic_cast<IONode*>(scan->prev());
		IFile *file = in->file();
		// ... continue ... ask file for DataStats
	} else {
		// ... continue ... ask cache for DataStats
	}

	return height; // @ returns 0
}

} } // namespace map::detail
