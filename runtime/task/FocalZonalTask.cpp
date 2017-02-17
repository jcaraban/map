/**
 * @file    FocalTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "FocalZonalTask.hpp"
#include "../Runtime.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

FocalZonalTask::FocalZonalTask(Group *group)
	: Task(group)
{
	createVersions();
}

void FocalZonalTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void FocalZonalTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	in_keys.clear();
	for (int i=0; i<inputList().size(); i++)
	{
		Node *node = inputList()[i];
		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		const int N = (node->numdim() == D0) ? 0 : is_input_of[i].is(FOCAL) ? 1 : 0;
		
		for (int y=-N; y<=N; y++) {
			for (int x=-N; x<=N; x++) {
				auto nbc = coord + Coord{x,y};
				HoldType hold_nbc = (any(nbc < 0) || any(nbc >= numblock())) ? HOLD_0 : hold;
				in_keys.push_back( std::make_tuple(Key(node,nbc),hold_nbc) );
			}
		}
	}
}

void FocalZonalTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
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
		int pos = list_position(done_block.node,inputList());
		const int N = (done_block.node->numdim() == D0) ? 0 : is_input_of[pos].is(FOCAL) ? 1 : 0;

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

int FocalZonalTask::selfInterDepends(Node *node, Coord coord) const {
	int pos = list_position(node,inputList());
	if (!is_input_of[pos].is(FOCAL) || node->numdim() == D0)
		return node->pattern() == FREE ? 0 : 1;
	
	// Focal inputs depend on their neighborhood
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

int FocalZonalTask::nextInterDepends(Node *node, Coord coord) const {
	return selfInterDepends(node,coord); // @ reusing selfInterDepends, but would need own code
}

int FocalZonalTask::selfIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal-Zonal do not present intra dependencies
}

int FocalZonalTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal-Zonal do not present intra dependencies
}

void FocalZonalTask::preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	// @ same code than ZonalTask::
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

			cl_int clerr = clEnqueueFillBuffer(*que,b->scalar_page,&neutral.get(),neutral.datatype().sizeOf(),index,b->datatype().sizeOf(),0,nullptr,nullptr);
			cle::clCheckError(clerr);
		}
	}
}

void FocalZonalTask::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	// @ same code than ZonalTask::
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
			cl_int clerr = clEnqueueReadBuffer(*que,b->scalar_page,CL_TRUE,index,b->datatype().sizeOf(),&b->value.get(),0,nullptr,nullptr);
			cle::clCheckError(clerr);
		}
	}
}

void FocalZonalTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}

} } // namespace map::detail
