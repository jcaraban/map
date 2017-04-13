/**
 * @file    ZonalTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "ZonalTask.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

/*********
   Zonal
 *********/

ZonalTask::ZonalTask(Group *group)
	: Task(group)
{
	createVersions();
}

void ZonalTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void ZonalTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void ZonalTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void ZonalTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);	
}

void ZonalTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void ZonalTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
		notifyAll(job_vec);
	else // Case when prev=D2, self=D2
		notify(done_block.coord,job_vec);
}

int ZonalTask::prevInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int ZonalTask::nextInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int ZonalTask::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Zonal do not present intra dependencies
}

int ZonalTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Zonal do not present intra dependencies
}

void ZonalTask::preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	Task::preCompute(coord,in_blk,out_blk);

	const Version *ver = version(DEV_ALL,""); // Any device, No detail
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

void ZonalTask::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	Task::postCompute(coord,in_blk,out_blk);

	const Version *ver = version(DEV_ALL,""); // Any device, No detail
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

			if (b->key.node->pattern().is(ZONAL)) {
				auto *red = dynamic_cast<ZonalReduc*>(b->key.node);
				std::lock_guard<std::mutex> lock(mtx); // thread-safe
				red->value = red->type.apply(red->value,b->value);
		//std::cout << "red: " << red->id << " val: " << red->value << std::endl;
			}
		}
	}
}

void ZonalTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}

} } // namespace map::detail
