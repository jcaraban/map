/**
 * @file    RadialTask.hpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_RADIAL_HPP_
#define MAP_RUNTIME_TASK_RADIAL_HPP_

#include "Task.hpp"
#include "../dag/RadialScan.hpp"


namespace map { namespace detail {

struct RadialTask : public Task
{
	RadialTask(Program &prog, Clock &clock, Config &conf, Cluster *cluster);

	void createVersions();

	void blocksToLoad(Job job, KeyList &in_key) const;
	void blocksToStore(Job job, KeyList &out_key) const;
	
	void initialJobs(std::vector<Job> &job_vec);
	void selfJobs(Job done_job, std::vector<Job> &job_vec);
	void nextJobs(Job done_job, std::vector<Job> &job_vec, bool end);
	
	int prevInterDepends(Node *node, Coord coord) const;
	int nextInterDepends(Node *node, Coord coord) const;
	int prevIntraDepends(Node *node, Coord coord) const;
	int nextIntraDepends(Node *node, Coord coord) const;
	
	void compute(Job job, const BlockList &in_blk, const BlockList &out_blk);
	
	Pattern pattern() const { return RADIAL; }

  // vars
	RadialScan *scan;
	Coord startb;
};

} } // namespace map::detail

#endif
