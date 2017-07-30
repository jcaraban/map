/**
 * @file    ScalarTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_SCALAR_HPP_
#define MAP_RUNTIME_TASK_SCALAR_HPP_

#include "Task.hpp"


namespace map { namespace detail {

/*
 *
 */
struct ScalarTask : public Task
{
	ScalarTask(Program &prog, Clock &clock, Config &conf, Cluster *cluster);

	void createVersions();

	void compute(Job job, const BlockList &in_blk, const BlockList &out_blk);

  // vars
	std::unordered_map<Node*,VariantType> hash;
};

} } // namespace map::detail

#endif
