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

#include "Job.hpp"
#include <vector>
#include <queue>
#include <unordered_set>
#include <mutex>
#include <condition_variable>


namespace map { namespace detail {

class Program; // Forward declaration
class Clock; // Forward declaration
class Config; // Forward declaration


/*
 *
 */
class Scheduler
{
  public:
  	Scheduler(Clock &clock, Config &conf);

  	void clear();
	void print();
	
	void initialJobs(const Program &prog);
	Job requestJob();
	void returnJob(Job job);

  private:
  	void waitForJob(std::unique_lock<std::mutex> &lock);
	void addJobs(const std::vector<Job> &job);

  private:
  	Clock &clock; // Aggregate
  	Config &conf; // Aggregate

  	std::vector<std::vector<Job>> job_vec_vec; // Allocates one job_vec per thread

	std::priority_queue<Job,std::vector<Job>,job_greater> job_queue; // Jobs ready to be issued
	std::unordered_set<Job,job_hash,job_equal> job_set; // Tracks uniqueness (necessary for loops)

	std::mutex mtx;
	std::condition_variable cv_job;
	int waiters_job;
	bool end;
};

} } // namespace map::detail

#endif
