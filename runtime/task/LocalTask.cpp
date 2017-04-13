/**
 * @file    LocalTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LocalTask.hpp"
#include "../Runtime.hpp"
#include "../dag/dag.hpp"


namespace map { namespace detail {

LocalTask::LocalTask(Group *group)
	: Task(group)
{
	createVersions();
}

void LocalTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	// All devices are accepted
	for (int i=0; i<env.deviceSize(); i++) {
		Version *ver = new Version(this,env.D(i),"");
		Runtime::getInstance().addVersion(ver); // Adds version to Runtime
		ver_list.push_back(ver);  // Adds version to Task
	}
}

void LocalTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void LocalTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void LocalTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);
}

void LocalTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void LocalTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
		notifyAll(job_vec);
	else // Case when prev=D2, self=D2
		notify(done_block.coord,job_vec);
}

int LocalTask::prevInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int LocalTask::nextInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int LocalTask::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Local does not present intra dependencies
}

int LocalTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Local does not present intra dependencies
}

void LocalTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = version(DEV_ALL,""); // Any device, No detail
	Task::computeVersion(coord,in_blk,out_blk,ver);
}

} } // namespace map::detail
