/**
 * @file    IdentityTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "IdentityTask.hpp"
#include "../Runtime.hpp"
#include "../dag/dag.hpp"


namespace map { namespace detail {

IdentityTask::IdentityTask(Group *group)
	: Task(group)
{
	createVersions();
}

void IdentityTask::createVersions() {
	return; // No versions needed for IdentityTask
}

void IdentityTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void IdentityTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void IdentityTask::initialJobs(std::vector<Job> &job_vec) {
	assert(0); // should never be a first task, shall it?
}

void IdentityTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void IdentityTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
		notifyAll(job_vec);
	else // Case when prev=D2, self=D2
		notify(done_block.coord,job_vec);
}

int IdentityTask::prevInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int IdentityTask::nextInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int IdentityTask::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Identity does not present intra dependencies
}

int IdentityTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Identity does not present intra dependencies
}

void IdentityTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	assert(in_blk.size() == out_blk.size());

	// @@ Swapping 'dev_mem' pointers, this might not work!

	for (int i=0; i<in_blk.size(); i++) {
		cl_mem aux = in_blk[i]->entry->dev_mem;
		in_blk[i]->entry->dev_mem = out_blk[i]->entry->dev_mem;
		out_blk[i]->entry->dev_mem = aux;
	}
}

} } // namespace map::detail
