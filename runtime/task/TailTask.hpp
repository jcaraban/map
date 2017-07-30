/**
 * @file    TailTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_TAIL_HPP_
#define MAP_RUNTIME_TASK_TAIL_HPP_

#include "Task.hpp"


namespace map { namespace detail {

/*
 *
 */
struct TailTask : public Task
{	
	TailTask(Program &prog, Clock &clock, Config &conf, Cluster *cluster);	

	void blocksToStore(Job job, KeyList &in_key) const;
	void askJobs(Job done_job, std::vector<Job> &job_vec);

  // vars
};

} } // namespace map::detail

#endif
