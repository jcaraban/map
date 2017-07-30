/**
 * @file    Task.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Task base class
 */

#ifndef MAP_RUNTIME_TASK_HPP_
#define MAP_RUNTIME_TASK_HPP_

#include "../dag/Cluster.hpp"
#include "../Job.hpp"
#include "../Version.hpp"
#include "../block/BlockList.hpp"
#include "../ThreadId.hpp"
#include "../../util/util.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <mutex>


namespace map { namespace detail {

class Program; // Forward declaration
class Clock; // Forward declaration
class Config; // Forward declaration

struct Task; // forward declaration
typedef std::vector<Task*> TaskList;


/*
 *
 */
struct Task
{
  // Constructors
	static Task* Factory(Program &prog, Clock &clock, Config &conf, Cluster *cluster);

	Task(Program &prog, Clock &clock, Config &conf, Cluster *cluster);
	virtual ~Task() { }; 
	
  // Methods
	int id() const;
	const Cluster* cluster() const;
	const NodeList& nodeList() const;
	const NodeList& inputList() const;
	const NodeList& outputList() const;
	const TaskList& prevList() const;
	const TaskList& nextList() const;
	const TaskList& backList() const;
	const TaskList& forwList() const;
	bool isPrev(const Task *task) const; // @ unnecessary?
	bool isNext(const Task *task) const; // @ unnecessary?
  // Meta
	NumDim numdim() const;
	const DataSize& datasize() const;
	const BlockSize& blocksize() const;
	const NumBlock& numblock() const;
	const GroupSize& groupsize() const;
	const NumGroup& numgroup() const;
  // Spatial
	virtual Pattern pattern() const;
	virtual const Mask& accuInputReach(Node *node, Coord coord) const;
	virtual const Mask& accuOutputReach(Node *node, Coord coord) const;
  // Versions
	virtual void createVersions();
	const VersionList& versionList() const;
	const Version* getVersion(DeviceType dev_type, GroupSize group_size, std::string detail) const;
  // Blocks
	virtual void blocksToLoad(Job job, KeyList &in_key) const;
	virtual void blocksToStore(Job job, KeyList &out_key) const;
  // Jobs
	virtual void initialJobs(std::vector<Job> &job_vec);
	virtual void askJobs(Job done_job, std::vector<Job> &job_vec);
	virtual void selfJobs(Job done_job, std::vector<Job> &job_vec);
	virtual void nextJobs(Job done_job, std::vector<Job> &job_vec, bool end); // @
	void notify(Job new_job, std::vector<Job> &job_vec);
	void notifyAll(Job new_job, std::vector<Job> &job_vec);
  // Dependencies
	virtual int prevDependencies(Coord coord) const;
	virtual int nextDependencies(Node *node, Coord coood) const;
	virtual int prevInterDepends(Node *node, Coord coord) const;
	virtual int nextInterDepends(Node *node, Coord coord) const;
	virtual int prevIntraDepends(Node *node, Coord coord) const;
	virtual int nextIntraDepends(Node *node, Coord coord) const;
	int nextInputDepends(Node *node, Coord coord) const; // @
  // Compute
	virtual void preLoad(Job job, const BlockList &in_blk, const BlockList &out_blk);
	virtual void preCompute(Job job, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postCompute(Job job, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postStore(Job job, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postWork(Job job, const BlockList &in_blk, const BlockList &out_blk); // @

	virtual void compute(Job job, const BlockList &in_blk, const BlockList &out_blk);
	virtual void computeVersion(Job job, const BlockList &in_blk, const BlockList &out_blk, const Version *ver);

	virtual void fixingValues(Job job, const BlockList &in_blk, const BlockList &out_blk);
	virtual void preForward(Job job, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postForward(Job job, const BlockList &in_blk, const BlockList &out_blk);

  // vars
	Program &prog; // Aggregate
  	Clock &clock; // Aggregate
  	Config &conf; // Aggregate

	Cluster *base_cluster; //!< Cluster that founded the task
	
	TaskList prev_list; //!< Prev tasks on which this one depends
	TaskList next_list; //!< Next tasks depending on this one
	TaskList back_list; //!< Back tasks 
	TaskList forw_list; //!< Forward tasks 

	VersionList ver_list; //!< List of versions required by the task
	
	std::unordered_map<Coord,int,coord_hash,coord_equal> dep_hash; // Structure holding the job dependencies met so far
	std::unordered_map<int,int> prev_jobs_count, self_jobs_count;//, next_jobs_count;
	ThreadId last; // @ per iter?

	std::unordered_map<Node*,Mask> accu_in_reach_of; // Accumulated 'spatial reach' of input nodes (e.g. Focal, Radial)
	std::unordered_map<Node*,Mask> accu_out_reach_of; // Accumulated 'spatial reach' of output nodes (e.g. Spread)

	mutable std::mutex mtx;

  // helper vars
	std::vector<TaskList> next_of_out; //!< Next tasks depending on the respective out-node
	std::vector<Pattern> is_input_of; //!< Pattern of the nodes using the respective input-node

	std::vector<std::unordered_map<Node*,Block*>> forward_list;
};

} } // namespace map::detail

#endif
