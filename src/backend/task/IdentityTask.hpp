/**
 * @file    IdentityTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_IDENTITY_HPP_
#define MAP_RUNTIME_TASK_IDENTITY_HPP_

#include "../Task.hpp"


namespace map { namespace detail {

/*
 *
 */
struct IdentityTask : public Task
{
	IdentityTask(Program &prog, Clock &clock, Config &conf, Cluster *cluster);

	void createVersions();

	void compute(Job job, const BlockList &in_blk, const BlockList &out_blk);
};

} } // namespace map::detail

#endif
