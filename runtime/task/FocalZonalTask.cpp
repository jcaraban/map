/**
 * @file    FocalTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "FocalZonalTask.hpp"
#include "../Runtime.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

FocalZonalTask::FocalZonalTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{ }
/*
void FocalZonalTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void FocalZonalTask::blocksToLoad(Coord coord, KeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void FocalZonalTask::blocksToStore(Coord coord, KeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void FocalZonalTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);
}

void FocalZonalTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void FocalZonalTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
	{
		notifyAll(job_vec);
	}
	else // Case when prev=D2, self=D2
	{
		auto reach = spatial_reach_of.find(done_block.node)->second;
		auto space = reach.blockSpace(blocksize());

		for (auto offset : space) {	
			auto nbc = done_block.coord + offset;
			if (all(in_range(nbc,numblock()))) {
				notify(nbc,job_vec);
			}
		}
	}
}

int FocalZonalTask::prevInterDepends(Node *node, Coord coord) const {
	auto reach = spatial_reach_of.find(node)->second;
	auto space = reach.blockSpace(blocksize());
	int depend = 0;

	for (auto offset : space) {
		Coord nbc = coord + offset;
		if (all(in_range(nbc,numblock()))) {
			depend += node->pattern()==FREE ? 0 : 1;
		}
	}
	
	return depend;
}

int FocalZonalTask::nextInterDepends(Node *node, Coord coord) const {
	return prevInterDepends(node,coord);
}

int FocalZonalTask::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal-Zonal do not present intra dependencies
}

int FocalZonalTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal-Zonal do not present intra dependencies
}

void FocalZonalTask::preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	// @ same code than ZonalTask::
	Task::preCompute(coord,in_blk,out_blk);

	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
	const Config &conf = Runtime::getConfig();

	// Fills output-zonal-scalars with neutral value
	int sidx = 0;
	for (auto &b : out_blk) {
		if (b->holdtype() == HOLD_1)
		{
			ReductionType rtype;
			auto *zn = dynamic_cast<ZonalReduc*>(b->key.node);
			if(zn != nullptr)
				rtype = zn->type;
			auto *sn = dynamic_cast<Scalar*>(b->key.node);
			if(sn != nullptr) {
				auto *zn = dynamic_cast<ZonalReduc*>(sn->prev());
				assert(zn != nullptr);
				rtype = zn->type;
			}
			assert(rtype != NONE_REDUCTION);

			int index = sizeof(double)*(conf.max_out_block*Tid.rnk() + sidx++);
			VariantType neutral = rtype.neutral(b->datatype());
			b->value = neutral; // necessary to set the datatype

			cl_int clerr = clEnqueueFillBuffer(*que,b->scalar_page,&neutral.ref(),neutral.datatype().sizeOf(),index,b->datatype().sizeOf(),0,nullptr,nullptr);
			cle::clCheckError(clerr);
		}
	}
}

void FocalZonalTask::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	// @ same code than ZonalTask::
	Task::postCompute(coord,in_blk,out_blk);

	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
	const Config &conf = Runtime::getConfig();

	// Transfers back the output-zonal-scalars
	int sidx = 0;
	for (auto &b : out_blk) {
		if (b->holdtype() == HOLD_1)
		{
			int index = sizeof(double)*(conf.max_out_block*Tid.rnk() + sidx++);
			cl_int clerr = clEnqueueReadBuffer(*que,b->scalar_page,CL_TRUE,index,b->datatype().sizeOf(),&b->value.ref(),0,nullptr,nullptr);
			cle::clCheckError(clerr);
		}
	}
}

void FocalZonalTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}
*/
} } // namespace map::detail
