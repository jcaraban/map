/**
 * @file    SpreadTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * To be filled by some specialized spreading operation? e.g. 'flow accumulation'
 */

#ifndef MAP_RUNTIME_TASK_SPREAD_HPP_
#define MAP_RUNTIME_TASK_SPREAD_HPP_

#include "Task.hpp"
#include "../dag/SpreadScan.hpp"
#include <array>


namespace map { namespace detail {

typedef std::array<std::array<bool,3>,3> stable_vec;

struct SpreadTask : public Task
{	
	SpreadTask(Program &prog, Clock &clock, Config &conf, Group *group);	

	void createVersions();

	void blocksToLoad(Job job, KeyList &in_key) const;
	void blocksToStore(Job job, KeyList &out_key) const;

	void initialJobs(std::vector<Job> &job_vec);
	void askJobs(Job done_job, std::vector<Job> &job_vec);
	void selfJobs(Job done_job, std::vector<Job> &job_vec);
	void nextJobs(Job done_job, std::vector<Job> &job_vec);

	int prevInterDepends(Node *node, Coord coord) const;
	int nextInterDepends(Node *node, Coord coord) const;
	int prevIntraDepends(Node *node, Coord coord) const;
	int nextIntraDepends(Node *node, Coord coord) const;

	void compute(Job job, const BlockList &in_blk, const BlockList &out_blk);
	
	Pattern pattern() const { return SPREAD; }

  // vars
	SpreadScan *scan;
};

} } // namespace map::detail

#endif
