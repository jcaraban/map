/**
 * @file    IdentityTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "IdentityTask.hpp"
#include "../Runtime.hpp"
#include "../dag/dag.hpp"


namespace map { namespace detail {

IdentityTask::IdentityTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{ }

void IdentityTask::createVersions() {
	return; // No versions needed for IdentityTask
}

void IdentityTask::compute(Job job, const BlockList &in_blk, const BlockList &out_blk) {
	assert(in_blk.size() == out_blk.size());

	auto all_pred = [&](Block *b){ return b->isFixed() || b->isForward(); };
	assert(std::all_of(out_blk.begin(),out_blk.end(),all_pred));

	// All blocks in an IdentityTask must forward, 
	// This happens in ::pre/postForward, here we only assert()
	
	clock.incr(NOT_COMPUTED);
}

} } // namespace map::detail
