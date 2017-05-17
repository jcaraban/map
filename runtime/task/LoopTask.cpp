/**
 * @file    LoopTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LoopTask.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

LoopTask::LoopTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{
	assert(inputList().size() % 2 == 0);
	self_jobs_count = -1;

	for (auto node : group->nodeList()) {
		if (node->pattern().is(LOOP)) {
			this->cond_node = dynamic_cast<LoopCond*>(node);
			assert(this->cond_node != nullptr);
			break;
		}
	}
}

void LoopTask::blocksToLoad(Coord coord, KeyList &in_keys) const {
	in_keys.clear();

	for (auto node : inputList()) {
		if (left_input && node->pattern().isNot(HEAD))
			continue;
		if (right_output && node->pattern().is(HEAD))
			continue;

		auto reach = accuInputReach(node,coord);
		auto space = reach.blockSpace(blocksize());

		for (auto offset : space) {	
			Coord nbc = coord + offset;
			HoldType hold = node->holdtype(nbc);
			Depend dep = node->isInput() ? nextInputDepends(node,nbc) : -1;

			in_keys.push_back( std::make_tuple(Key(node,nbc),hold,dep) );
		}
	}
}

void LoopTask::blocksToStore(Coord coord, KeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);
}

void LoopTask::initialJobs(std::vector<Job> &job_vec) {
	assert(0); // Should never be called
}

void LoopTask::askJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);

	// Asks next-tasks for their next-jobs, a.k.a inter-dependencies (all Op)
	for (auto next_task : this->nextList()) {
		if (left_output && next_task->pattern().isNot(TAIL))
			continue;
		if (right_output && next_task->pattern().is(TAIL))
			continue;
		auto common_nodes = inner_join(this->outputList(),next_task->inputList());
		for (auto node : common_nodes) {
			assert(node->pattern().is(SWITCH));
			Key key = Key(node,done_job.coord);
			next_task->nextJobs(key,job_vec);
		}
	}
}

void LoopTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	return; // nothing to do
}

void LoopTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	Task::nextJobs(done_block,job_vec);

	if (done_block.node->pattern().is(HEAD))
	{
		left_input = (!left_input && !right_input) ? true : left_input;
		assert(left_input && not right_input);
	}
	else // node->pattern isNot HEAD
	{
		right_input = (!left_input && !right_input) ? true : right_input;
		assert(not left_input && right_input);
	}
}

int LoopTask::prevDependencies(Coord coord) const {
	int dep = Task::prevDependencies(coord);
	assert(dep % 2 == 0);
	return dep / 2;
}

void LoopTask::postStore(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	assert(cond_node != nullptr);

	Block *cond_blk = nullptr;
	for (auto blk : out_blk)
		cond_blk = (blk->key.node == cond_node) ? blk : cond_blk;
	assert(cond_blk != nullptr);

	if (cond_blk->value)
		right_output = true;
	else
		left_output = true;
}

void LoopTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, No detail
	assert(ver != nullptr);

	auto all_pred = [&](Block *b){ return b->fixed || b->key.node->canForward(); };
	if (std::all_of(out_blk.begin(),out_blk.end(),all_pred)) {
		clock.incr(NOT_COMPUTED);
		return; // All output blocks are fixed, no need to compute
	}

	computeVersion(coord,in_blk,out_blk,ver);
}

} } // namespace map::detail
