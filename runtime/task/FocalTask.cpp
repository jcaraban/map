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

FocalTask::FocalTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{ }
/*
void FocalTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void FocalTask::blocksToLoad(Coord coord, KeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void FocalTask::blocksToStore(Coord coord, KeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void FocalTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);
}

void FocalTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void FocalTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	Task::nextJobs(done_block,job_vec);
}

int FocalTask::prevInterDepends(Node *node, Coord coord) const {
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

int FocalTask::nextInterDepends(Node *node, Coord coord) const {
	return prevInterDepends(node,coord); // prevInter != nextInter in Radial
}

int FocalTask::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal do not present intra dependencies
}

int FocalTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Focal do not present intra dependencies
}

void FocalTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}
*/
} } // namespace map::detail
