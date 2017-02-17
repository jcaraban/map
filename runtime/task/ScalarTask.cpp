/**
 * @file    ScalarTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "ScalarTask.hpp"
#include "../Runtime.hpp"
#include "../dag/dag.hpp"


namespace map { namespace detail {

ScalarTask::ScalarTask(Group *group)
	: Task(group)
{
	createVersions();
}

void ScalarTask::createVersions() {
	return; // No versions needed for ScalarTask
}

void ScalarTask::blocksToLoad(Coord coord, InKeyList &in_keys) const {
	// @ Does the same than Task::
	in_keys.clear();
	for (auto in_node : inputList())
		in_keys.push_back( std::make_tuple(Key(in_node,coord),HOLD_1) );
}

void ScalarTask::blocksToStore(Coord coord, OutKeyList &out_keys) const {
	// @ Only difference with Task:: is the 0 dependencies
	out_keys.clear();
	for (auto out_node : outputList())
		out_keys.push_back( std::make_tuple(Key(out_node,coord),HOLD_1,0) );
}

void ScalarTask::initialJobs(std::vector<Job> &job_vec) {
	job_vec.push_back( Job(this,Coord{0,0}) ); // Pushes 1 single job
}

void ScalarTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void ScalarTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe
	if (--prev_jobs_count == 0)
		job_vec.push_back( Job(this,Coord{0,0}) ); // Pushes 1 single job
	assert(prev_jobs_count >= 0);
}

int ScalarTask::selfInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int ScalarTask::nextInterDepends(Node *node, Coord coord) const {
	return node->pattern() == FREE ? 0 : 1;
}

int ScalarTask::selfIntraDepends(Node *node, Coord coord) const {
	return 0; // Scalar do not present intra dependencies
}

int ScalarTask::nextIntraDepends(Node *node, Coord coord) const {
	return 0; // Scalar do not present intra dependencies
}

void ScalarTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	assert(out_blk.size() == 1);

	this->in_blk = in_blk;
	this->out_blk = out_blk;

	// Goes up, node by node, starting backward, computing the scalar values
	for (auto i=outputList().rbegin(); i!=outputList().rend(); i++)
		(*i)->accept(this);
}

/*********
   Visit
 *********/
// @
bool ScalarTask::is_input(Node *node) {
	if (is_included(node,group()->inputList())) {
		// Visited as INPUT
		auto it = std::find_if(in_blk.begin(),in_blk.end(),[&](Block *blk){ return blk->key.node == node; });
		assert(it != in_blk.end());
		variant = (*it)->value;
		return true;
	}
	return false;
}
// @
bool ScalarTask::is_output(Node *node) {
	if (is_included(node,group()->outputList())) {
		// Visited as OUTPUT
		auto it = std::find_if(out_blk.begin(),out_blk.end(),[&](Block *blk){ return blk->key.node == node; });
		assert(it != out_blk.end());
		(*it)->value = variant;
		return true;
	}
	return false;
}

void ScalarTask::visit(Constant *node) {
	if (is_input(node)) return;

	variant = node->cnst;

	node->value = variant;
	is_output(node);
}

void ScalarTask::visit(Cast *node) {
	if (is_input(node)) return;

	node->prev()->accept(this);
	auto prev = variant;
	variant = prev.convert(node->type);

	node->value = variant;
	is_output(node);
}

void ScalarTask::visit(Unary *node) {
	if (is_input(node)) return;

	node->prev()->accept(this);
	auto prev = variant;
	variant = node->type.apply(prev);

	node->value = variant;
	is_output(node);
}

void ScalarTask::visit(Binary *node) {
	if (is_input(node)) return;

	node->left()->accept(this);
	auto left = variant;
	node->right()->accept(this);
	auto right = variant;
	variant = node->type.apply(left,right);

	node->value = variant;
	is_output(node);
}

void ScalarTask::visit(Conditional *node) {
	if (is_input(node)) return;

	node->cond()->accept(this);
	auto cond = variant;
	node->left()->accept(this);
	auto left = variant;
	node->right()->accept(this);
	auto right = variant;
	variant = ( cond.convert(B8).get<B8>() ) ? left : right;

	node->value = variant;
	is_output(node);
}

void ScalarTask::visit(Diversity *node) {
	if (is_input(node)) return;

	std::vector<VariantType> varvec;
	for (auto prev : node->prevList()) {
		prev->accept(this);
		varvec.push_back(variant);
	}
	variant = node->type.apply(varvec);

	node->value = variant;
	is_output(node);
}

void ScalarTask::visit(Scalar *node) {
	if (is_input(node)) return;

	node->prev()->accept(this);

	node->Node::value = variant;
	is_output(node);
}

void ScalarTask::visit(ZonalReduc *node) {
	if (is_input(node)) return;
	assert(0);
}

} } // namespace map::detail
