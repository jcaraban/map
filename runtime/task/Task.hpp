/**
 * @file    Task.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Task base class
 *
 * TODO: GPU shared memory should be dynamically allocated
 */

#ifndef MAP_RUNTIME_TASK_HPP_
#define MAP_RUNTIME_TASK_HPP_

#include "../Job.hpp"
#include "../Version.hpp"
#include "../Block.hpp"
#include "../ThreadId.hpp"
#include "../dag/Group.hpp"
#include "../../util/util.hpp"
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <mutex>


namespace map { namespace detail {

struct Task; // forward declaration

/*********
   Utils
 *********/

typedef std::vector<Task*> TaskList;

struct task_hash {
	std::size_t operator()(const Task *t) const;
};

/********
   Task
 ********/

/*
 *
 */
struct Task
{
  // Constructors
	static Task* Factory(Group *group);

	Task(Group *group);
	virtual ~Task() { }; 
	
  // methods
	int id() const;
	
	const Group* group() const;
	const NodeList& nodeList() const;
	const NodeList& inputList() const;
	const NodeList& outputList() const;
	const TaskList& prevList() const;
	const TaskList& nextList() const;
	bool isPrev(const Task *task) const; // @ unnecessary?
	bool isNext(const Task *task) const; // @ unnecessary?

	NumDim numdim() const;
	const DataSize& datasize() const;
	const BlockSize& blocksize() const;
	const NumBlock& numblock() const;

	virtual void createVersions() = 0;
	const VersionList& versionList() const;
	const Version* version(DeviceType dev_type, std::string detail) const;

	virtual void blocksToLoad(Coord coord, InKeyList &in_keys) const;
	virtual void blocksToStore(Coord coord, OutKeyList &out_keys) const;

	virtual void initialJobs(std::vector<Job> &job_vec);
	virtual void askJobs(Job done_job, std::vector<Job> &job_vec);
	virtual void selfJobs(Job done_job, std::vector<Job> &job_vec) = 0;
	virtual void nextJobs(Key done_block, std::vector<Job> &job_vec) = 0;
	void notify(Coord coord, std::vector<Job> &job_vec);
	void notifyAll(std::vector<Job> &job_vec);

	int selfDependencies(Coord coord) const;
	int nextDependencies(Node *node, Coord coood) const;
	virtual int selfInterDepends(Node *node, Coord coord) const = 0;
	virtual int nextInterDepends(Node *node, Coord coord) const = 0;
	virtual int selfIntraDepends(Node *node, Coord coord) const = 0;
	virtual int nextIntraDepends(Node *node, Coord coord) const = 0;

	virtual void preLoad(Coord coord);
	virtual void preCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postCompute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	virtual void postStore(Coord coord);

	virtual void compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	virtual void computeVersion(Coord coord, const BlockList &in_blk, const BlockList &out_blk, const Version *ver);
	
	virtual Pattern pattern() const = 0;

  // vars
	Group *base_group; //!< Group that founded the task
	
	TaskList prev_list; //!< Prev tasks on which this one depends
	TaskList next_list; //!< Next tasks depending on this one

	VersionList ver_list; //!< List of versions required by the task
	
	std::unordered_map<Coord,int,coord_hash,coord_equal> dep_hash; // Structure holding the job dependencies met so far
	int prev_jobs_count, self_jobs_count;//, next_jobs_count;
	ThreadId last;

	mutable std::mutex mtx;

  // cached vars
	std::vector<TaskList> next_of_out; //!< Next tasks depending on the respective out-node
	std::vector<Pattern> is_input_of; //!< Pattern of the nodes using the respective input-node
};

} } // namespace map::detail

#endif
