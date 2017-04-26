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

ZonalTask::ZonalTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{ }
/*
void ZonalTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void ZonalTask::blocksToLoad(Coord coord, KeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void ZonalTask::blocksToStore(Coord coord, KeyList &out_keys) const {
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
}

void ZonalTask::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	Task::postCompute(coord,in_blk,out_blk);
}

void ZonalTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}
*/
} } // namespace map::detail
