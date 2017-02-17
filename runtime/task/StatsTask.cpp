/**
 * @file    StatsTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "StatsTask.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

/*********
   Stats
 *********/

StatsTask::StatsTask(Group *group)
	: Task(group)
{
	// @ TODO: fix this somehow
	for (auto node : group->nodeList()) {
		auto *stats = dynamic_cast<Stats*>(node);
		if (stats != nullptr) {
			this->stats = stats;
			break;
		}
	}

	createVersions();
}

void StatsTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void StatsTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void StatsTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);

	// Adds max, mean, min, std output blocks
	out_keys.push_back( std::make_tuple(Key(stats->max(),coord),HOLD_1,0) );
	out_keys.push_back( std::make_tuple(Key(stats->min(),coord),HOLD_1,0) );

	assert(out_keys.size() == 3);
}

void StatsTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);	
}

void StatsTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void StatsTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
		notifyAll(job_vec);
	else // Case when prev=D2, self=D2
		notify(done_block.coord,job_vec);
}

int StatsTask::selfInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int StatsTask::nextInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int StatsTask::selfIntraDepends(Node *node, Coord coord) const {
	return 0; // Stats do not present intra dependencies
}

int StatsTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Stats do not present intra dependencies
}

void StatsTask::preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	Task::preCompute(coord,in_blk,out_blk);

	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
	const Config &conf = Runtime::getConfig();

	assert(out_blk.size() == 3);

	// Fills with neutral value
	int sidx = 0;
	ReductionEnum rtypes[] = {MAX,MIN};

	for (int i=0; i<2; i++) {
		Block *b = out_blk[i+1];

		ReductionType rtype;
		auto *zn = dynamic_cast<ZonalReduc*>(b->key.node);
		if(zn != nullptr)
			rtype = zn->type;

		int index = sizeof(double)*(conf.max_out_block*Tid.rnk() + sidx++);
		VariantType neutral = rtype.neutral(stats->datatype());
		b->value = neutral;

		cl_int clerr = clEnqueueFillBuffer(*que,b->scalar_page,&neutral.get(),neutral.datatype().sizeOf(),index,b->datatype().sizeOf(),0,nullptr,nullptr);
		cle::clCheckError(clerr);
	}
}

void StatsTask::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	Task::postCompute(coord,in_blk,out_blk);

	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
	const Config &conf = Runtime::getConfig();

	assert(out_blk.size() == 3);

	// Transfers back the output-zonal-scalars
	int sidx = 0;
	for (int i=0; i<2; i++) {
		Block *b = out_blk[i+1];

		int index = sizeof(double)*(conf.max_out_block*Tid.rnk() + sidx++);
		cl_int clerr = clEnqueueReadBuffer(*que,b->scalar_page,CL_TRUE,index,b->datatype().sizeOf(),&b->value.get(),0,nullptr,nullptr);
		cle::clCheckError(clerr);
	}

	auto max = out_blk[1]->value;
	auto min = out_blk[2]->value;

	// Fills 'block' stats
	out_blk[0]->stats.active = true;
	out_blk[0]->stats.max = max.get();
	out_blk[0]->stats.min = min.get();
	out_blk[0]->fixed = (max == min);
	if (out_blk[0]->fixed) {
		out_blk[0]->value = max;
	}

	// Fills 'node' stats
	stats->stats.maxb[proj(coord,stats->numblock())] = max.get();
	stats->stats.minb[proj(coord,stats->numblock())] = min.get();

	// Writes global statistics
	if (Tid == last) {
		; // move this to a postFunction after askJobs is called?
	}
}

void StatsTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}

} } // namespace map::detail
