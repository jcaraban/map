/**
 * @file    IdentityTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_IDENTITY_HPP_
#define MAP_RUNTIME_TASK_IDENTITY_HPP_

#include "Task.hpp"
#include "../visitor/Visitor.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 *
 */
struct IdentityTask : public Task
{
	IdentityTask(Program &prog, Clock &clock, Config &conf, Group *group);

	void createVersions();

	void compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
