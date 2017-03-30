/**
 * @file    FocalTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: adding null keys leads to null blocks with null cl_mem. Null cl_mem are necessary to let the kernel know
 *       when it is in a corner case. Other option would be specializing different kernels, but this way is simpler.
 */

#include "FocalTask.hpp"
#include "../Runtime.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

/*********
   Focal
 *********/

FocalTask::FocalTask(Group *group)
	: Task(group)
{
	createVersions();
}

void FocalTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void FocalTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	in_keys.clear();
	
	for (int i=0; i<inputList().size(); i++)
	{
		Node *node = inputList()[i];
		const int N = is_input_of[i].is(FOCAL) ? 1 : 0;
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

void FocalTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void FocalTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);
}

void FocalTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void FocalTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
	{
		notifyAll(job_vec);
	}
	else // Case when prev=D2, self=D2
	{
		int pos = value_position(done_block.node,inputList());
		const int N = is_input_of[pos].is(FOCAL) ? 1 : 0;

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

int FocalTask::prevInterDepends(Node *node, Coord coord) const {
	int pos = value_position(node,inputList());
	if (!is_input_of[pos].is(FOCAL))
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

int FocalTask::nextInterDepends(Node *node, Coord coord) const {
	return prevInterDepends(node,coord); // @ reusing prevInterDepends, but would need own code
}

int FocalTask::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal do not present intra dependencies
}

int FocalTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal do not present intra dependencies
}

void FocalTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}

} } // namespace map::detail
