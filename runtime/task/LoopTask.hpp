/**
 * @file    LoopTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_LOOP_HPP_
#define MAP_RUNTIME_TASK_LOOP_HPP_

#include "Task.hpp"
#include "../dag/LoopCond.hpp"


namespace map { namespace detail {

/*
 *
 */
struct LoopTask : public Task
{	
	LoopTask(Program &prog, Clock &clock, Config &conf, Group *group);	

	void blocksToLoad(Job job, KeyList &in_key) const;
	void blocksToStore(Job job, KeyList &out_key) const;

	void initialJobs(std::vector<Job> &job_vec);
	void askJobs(Job done_job, std::vector<Job> &job_vec);
	void selfJobs(Job done_job, std::vector<Job> &job_vec);
	void nextJobs(Job done_job, std::vector<Job> &job_vec, bool end);

	int prevDependencies(Coord coord) const;

	void postStore(Job job, const BlockList &in_blk, const BlockList &out_blk);
	
	void preForward(Job job, const BlockList &in_blk, const BlockList &out_blk);

  // vars
	LoopCond *cond_node;
	MergeList merge_list;
	SwitchList switch_list;

	std::unordered_map<Job,bool,job_hash,job_equal> cycling_input;
	std::unordered_map<Job,bool,job_hash,job_equal> cycling_output;
};

} } // namespace map::detail

#endif
