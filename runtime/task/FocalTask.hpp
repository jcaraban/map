/**
 * @file    FocalTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_FOCAL_HPP_
#define MAP_RUNTIME_TASK_FOCAL_HPP_

#include "Task.hpp"


namespace map { namespace detail {

struct FocalTask : public Task
{
	FocalTask(Group *group);

	void createVersions();

	void blocksToLoad(Coord coord, InKeyList &in_keys) const;
	void blocksToStore(Coord coord, OutKeyList &out_keys) const;

	void initialJobs(std::vector<Job> &job_vec);
	void selfJobs(Job done_job, std::vector<Job> &job_vec);
	void nextJobs(Key done_block, std::vector<Job> &job_vec);
	
	int prevInterDepends(Node *node, Coord coord) const;
	int nextInterDepends(Node *node, Coord coord) const;
	int prevIntraDepends(Node *node, Coord coord) const;
	int nextIntraDepends(Node *node, Coord coord) const;

	void compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	
	Pattern pattern() const { return FOCAL; }
};

} } // namespace map::detail

#endif
