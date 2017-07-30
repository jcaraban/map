/**
 * @file    ScalarTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "ScalarTask.hpp"
#include "../Runtime.hpp"
#include "../dag/dag.hpp"


namespace map { namespace detail {

ScalarTask::ScalarTask(Program &prog, Clock &clock, Config &conf, Cluster *cluster)
	: Task(prog,clock,conf,cluster)
{
	for (auto in : inputList())
		assert(in->numdim() == D0);
	for (auto out : outputList())
		assert(out->numdim() == D0);
	assert(this->numdim() == D0);
}

void ScalarTask::createVersions() {
	return; // No versions needed for ScalarTask
}

//void ScalarTask::preCompute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
//	return; // nothing to do
//}

//void ScalarTask::postCompute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
//	return; // nothing to do
//}

void ScalarTask::compute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	assert(in_blk.size() == inputList().size());
	assert(out_blk.size() == outputList().size());
	assert(job.coord.size() == 0);

	// Loading scalar inputs
	for (int i=0; i<in_blk.size(); i++) {
		Node *node = inputList()[i];
		Block *blk = in_blk[i];

		assert(node == blk->node());
		assert(node->value.datatype() != NONE_DATATYPE);
		assert(node->value == blk->getValue());

		hash[node] = blk->getValue();
//std::cout << "iblk " << node->id << ":\t" << blk->getValue() << std::endl;
	}
	
	// Actual scalar computation, in forward order
	for (auto node : nodeList()) {
		node->computeScalar(hash);
		node->value = hash.find(node)->second;
//std::cout << "  scalar " << node->id << ":\t" << node->value << std::endl;
	}

	// Storing scalar outputs
	for (int i=0; i<out_blk.size(); i++) {
		Node *node = outputList()[i];
		Block *blk = out_blk[i];

		assert(node == blk->node());
		assert(node->value.datatype() != NONE_DATATYPE);
		assert(blk->getValue().datatype() == NONE_DATATYPE);

		blk->fixValue( hash.find(node)->second );
	}
}

} } // namespace map::detail
