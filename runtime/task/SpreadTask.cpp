/**
 * @file    SpreadTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "SpreadTask.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

/*************
   Spread
 *************/

SpreadTask::SpreadTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{
	assert(0);
}

void SpreadTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	
	assert(0);
}

void SpreadTask::blocksToLoad(Job job, KeyList &in_key) const {
	in_key.clear();
	
	assert(0);
}

void SpreadTask::blocksToStore(Job job, KeyList &out_key) const {
	out_key.clear();

	assert(0);
}

void SpreadTask::initialJobs(std::vector<Job> &job_vec) {
	assert(0);
}

void SpreadTask::askJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);	

	assert(0);
}

void SpreadTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);

	assert(0);
}

void SpreadTask::nextJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(0);
}

int SpreadTask::prevInterDepends(Node *node, Coord coord) const {
	assert(0);
}

int SpreadTask::nextInterDepends(Node *node, Coord coord) const {
	assert(0);
}

int SpreadTask::prevIntraDepends(Node *node, Coord coord) const {
	assert(0);
}

int SpreadTask::nextIntraDepends(Node *node, Coord coord) const {
	assert(0);
}

void SpreadTask::compute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	assert(0);
}

} } // namespace map::detail
