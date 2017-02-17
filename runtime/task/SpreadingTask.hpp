/**
 * @file    SpreadingTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: atm Spreading-GPU is done with an specific SpreadingTask, in the future it could be composed by a succession of Focals
 */

#ifndef MAP_RUNTIME_TASK_SPREAD_HPP_
#define MAP_RUNTIME_TASK_SPREAD_HPP_

#include "Task.hpp"
#include "../dag/SpreadScan.hpp"
#include <array>


namespace map { namespace detail {

typedef std::array<std::array<bool,3>,3> stable_vec;

struct SpreadingTask : public Task
{	
	SpreadingTask(Group *group);	

	void createVersions();

	void blocksToLoad(Coord coord, InKeyList &in_keys) const;
	void blocksToStore(Coord coord, OutKeyList &out_keys) const;

	void initialJobs(std::vector<Job> &job_vec);
	void askJobs(Job done_job, std::vector<Job> &job_vec);
	void selfJobs(Job done_job, std::vector<Job> &job_vec);
	void nextJobs(Key done_block, std::vector<Job> &job_vec);

	int selfInterDepends(Node *node, Coord coord) const;
	int nextInterDepends(Node *node, Coord coord) const;
	int selfIntraDepends(Node *node, Coord coord) const;
	int nextIntraDepends(Node *node, Coord coord) const;

	void compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	
	Pattern pattern() const { return SPREAD; }

	void fillScanBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que);
	void fillSpreadBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que);
	void fillStableBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que);
	void swapSpreadBuffer(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que);
	stable_vec readStableVec(Coord coord, const BlockList &in_blk, const BlockList &out_blk, cle::Queue que);
	unsigned int height(Coord coord) const;

  // vars
	SpreadScan *scan;
	std::unordered_map<Coord,stable_vec,coord_hash,coord_equal> stable_hash;
	std::unordered_set<Coord,coord_hash,coord_equal> first_time; // @ Used to fill spread->spread() only the 'first time'
};

} } // namespace map::detail

#endif
