/**
 * @file    Task.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Task base class
 *
 * TODO: TASK UNIFICATION ... doing ...
 * TODO: selfJobs() should take a 'Key done_block', but atm it is only used for Radial and is ok
 */

#ifndef MAP_RUNTIME_TASK_HPP_
#define MAP_RUNTIME_TASK_HPP_

#include "../dag/Group.hpp"
#include "../Job.hpp"
#include "../Version.hpp"
#include "../Block.hpp"
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
	static Task* Factory(Program &prog, Clock &clock, Config &conf, Group *group);

	Task(Program &prog, Clock &clock, Config &conf, Group *group);
	virtual ~Task() { }; 
	
  // Methods
	int id() const;
	const Group* group() const;
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
  // Spatial
	virtual Pattern pattern() const;
	virtual const Mask& inputReach(Node *node, Coord coord) const;
	virtual const Mask& outputReach(Node *node, Coord coord) const;
  // Versions
	virtual void createVersions();
	const VersionList& versionList() const;
	const Version* getVersion(DeviceType dev_type, GroupSize group, std::string detail) const;
  // Blocks
	virtual void blocksToLoad(Coord coord, KeyList &in_keys) const;
	virtual void blocksToStore(Coord coord, KeyList &out_keys) const;
  // Jobs
	virtual void initialJobs(std::vector<Job> &job_vec);
	virtual void askJobs(Job done_job, std::vector<Job> &job_vec);
	virtual void selfJobs(Job done_job, std::vector<Job> &job_vec);
	virtual void nextJobs(Key done_block, std::vector<Job> &job_vec);
	void notify(Coord coord, std::vector<Job> &job_vec);
	void notifyAll(std::vector<Job> &job_vec);
  // Dependencies
	int prevDependencies(Coord coord) const;
	int nextDependencies(Node *node, Coord coood) const;
	virtual int prevInterDepends(Node *node, Coord coord) const;
	virtual int nextInterDepends(Node *node, Coord coord) const;
	virtual int prevIntraDepends(Node *node, Coord coord) const;
	virtual int nextIntraDepends(Node *node, Coord coord) const;
	int nextInputDepends(Node *node, Coord coord) const; // @
  // Compute
	virtual void preLoad(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	virtual void preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postStore(Coord coord, const BlockList &in_blk, const BlockList &out_blk);

	virtual void compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	virtual void computeVersion(Coord coord, const BlockList &in_blk, const BlockList &out_blk, const Version *ver);

  // vars
	Program &prog; // Aggregate
  	Clock &clock; // Aggregate
  	Config &conf; // Aggregate

	Group *base_group; //!< Group that founded the task
	
	TaskList prev_list; //!< Prev tasks on which this one depends
	TaskList next_list; //!< Next tasks depending on this one
	TaskList back_list; //!< Back tasks 
	TaskList forw_list; //!< Forward tasks 

	VersionList ver_list; //!< List of versions required by the task
	
	std::unordered_map<Coord,int,coord_hash,coord_equal> dep_hash; // Structure holding the job dependencies met so far
	int prev_jobs_count, self_jobs_count;//, next_jobs_count;
	ThreadId last;

	std::unordered_map<Node*,Mask> accu_reach_of; // Accumulated 'spatial reach' of input nodes for a job to execute

	mutable std::mutex mtx;

  // cached vars
	std::vector<TaskList> next_of_out; //!< Next tasks depending on the respective out-node
	std::vector<Pattern> is_input_of; //!< Pattern of the nodes using the respective input-node
};

} } // namespace map::detail

#endif
