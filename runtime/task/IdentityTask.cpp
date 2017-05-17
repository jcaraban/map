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

void IdentityTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	assert(in_blk.size() == out_blk.size());

	// @@ Swapping 'dev_mem' pointers, this might not work!
	// definitely doesn't work if 'input' has several dependencies

	//for (int i=0; i<in_blk.size(); i++) {
	for (auto iblk : in_blk) {
		Block *oblk = nullptr;
		for (auto b : out_blk)
			oblk = (b->key.node->prevList()[0] == iblk->key.node) ? b : oblk;

		if (iblk->entry && oblk->entry) {
			std::swap( iblk->entry->dev_mem, oblk->entry->dev_mem );
		} else if (iblk->fixed) {
			oblk->fixed = true;
			oblk->value = iblk->value;
		} else {
			assert(0);
		}
	}
}

} } // namespace map::detail
