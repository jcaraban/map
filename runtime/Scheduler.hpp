/**
 * @file    Scheduler.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: the scheduling would be more efficient if the jobs are sorted in the queue near by their dependencies
 *       e.g. for two series of conv in parallel, better compute a whole series first, instead of interleaving
 * TODO: it will be necessary to have individual queues per device to keep locality
 */

#ifndef MAP_RUNTIME_SCHEDULER_HPP_
#define MAP_RUNTIME_SCHEDULER_HPP_

#include "Config.hpp"
#include "Job.hpp"
#include <vector>
#include <queue>
#include <unordered_set>
#include <mutex>
#include <condition_variable>


namespace map { namespace detail {

class Program; // Forward declaration
class Clock; // Forward declaration


/*
 *
 */
class Scheduler
{
  public:
  	Scheduler(Program &prog, Clock &clock, Config &conf);

  	void clear();
	void print();
	
	void initialJobs();
	Job requestJob();
	void returnJob(Job job);

  private:
  	void waitForJob(std::unique_lock<std::mutex> &lock);
	void addJobs(const std::vector<Job> &job);

  private:
  	Program &prog; // Aggregate
  	Clock &clock; // Aggregate
  	Config &conf; // Aggregate

  	std::vector<std::vector<Job>> job_vec_vec; // Allocates one job_vec per thread

	std::priority_queue<Job,std::vector<Job>,job_cmp> job_queue; // Jobs ready to be issued
	std::unordered_set<Job,job_hash,job_cmp> job_set; // Tracks uniqueness (necessary for Spreading)

	std::mutex mtx;
	std::condition_variable cv_job;
	int waiters_job;
	bool end;
};

} } // namespace map::detail

#endif
