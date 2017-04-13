/**
 * @file    LoopTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_LOOP_HPP_
#define MAP_RUNTIME_TASK_LOOP_HPP_

#include "Task.hpp"
#include "../dag/LoopCond.hpp"
#include <array>


namespace map { namespace detail {

/*
 *
 */
struct LoopTask : public Task
{	
	LoopTask(Group *group);	

	void createVersions();

	void blocksToLoad(Coord coord, InKeyList &in_keys) const;
	void blocksToStore(Coord coord, OutKeyList &out_keys) const;

	void initialJobs(std::vector<Job> &job_vec);
	void askJobs(Job done_job, std::vector<Job> &job_vec);
	void selfJobs(Job done_job, std::vector<Job> &job_vec);
	void nextJobs(Key done_block, std::vector<Job> &job_vec);

	int prevInterDepends(Node *node, Coord coord) const;
	int nextInterDepends(Node *node, Coord coord) const;
	int prevIntraDepends(Node *node, Coord coord) const;
	int nextIntraDepends(Node *node, Coord coord) const;

	void compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	
	Pattern pattern() const;

  // vars
	Pattern pat;
	LoopCond *cond_node;
	MergeList merge_list;
	SwitchList switch_list;
	bool left_input, right_input; // what input side activated the task
	bool left_output, right_output; // what output side will be notified
};

} } // namespace map::detail

#endif
