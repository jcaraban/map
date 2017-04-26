/**
 * @file    ScalarTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "ScalarTask.hpp"
#include "../Runtime.hpp"
#include "../dag/dag.hpp"


namespace map { namespace detail {

ScalarTask::ScalarTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{
	for (auto in : inputList())
		assert(in->numdim() == D0);
	for (auto out : outputList())
		assert(out->numdim() == D0);
}

void ScalarTask::createVersions() {
	return; // No versions needed for ScalarTask
}
/*
void ScalarTask::blocksToLoad(Coord coord, KeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);
}

void ScalarTask::blocksToStore(Coord coord, KeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void ScalarTask::initialJobs(std::vector<Job> &job_vec) {
	job_vec.push_back( Job(this,Coord(0)) ); // Pushes 1 single job
}

void ScalarTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void ScalarTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	assert(done_block.node->numdim() == D0); // prev must be D0
	notify(Coord(0),job_vec);
}

int ScalarTask::prevInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int ScalarTask::nextInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int ScalarTask::prevIntraDepends(Node *node, Coord coord) const {
	return 0; // Scalar do not present intra dependencies
}

int ScalarTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Scalar do not present intra dependencies
}
*/

void ScalarTask::preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	return; // nothing to do
}

void ScalarTask::postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	return; // nothing to do
}

void ScalarTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	assert(in_blk.size() == inputList().size());
	assert(out_blk.size() == outputList().size());
	
	coord = {0,0};

	// Checking inputs, @ not necesary ?
	for (int i=0; i<in_blk.size(); i++) {
		Node *node = inputList()[i];
		Block *blk = in_blk[i];

		assert(node == blk->key.node);		
		//assert(node->value.datatype() != NONE_DATATYPE);
		//assert(node->value == blk->value);

		//node->value = blk->value;
		hash[{node,coord}] = blk->value;
	}
	
	// Actual scalar computation, in forward order
	for (auto node : nodeList()) {
		node->computeScalar(hash);
		node->value = hash.find({node,coord})->second;
	}
	
	// Checking outputs, @ not necesary ?
	for (int i=0; i<out_blk.size(); i++) {
		Node *node = outputList()[i];
		Block *blk = out_blk[i];

		assert(node == blk->key.node);
		//assert(node->value.datatype() != NONE_DATATYPE);
		//assert(blk->value.datatype() == NONE_DATATYPE);

		//blk->value = node->value;
		blk->value = hash.find({node,coord})->second;
	}
}

} } // namespace map::detail
