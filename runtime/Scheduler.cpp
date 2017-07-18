/**
 * @file    Scheduler.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: re-check 'conf.num_*' if the scheduler is ever made hierarchical
 * TODO: try two scheduling fronts and rotating them when the queue gets empty (i.e. after a last zonal job)
 */

#include "Scheduler.hpp"
#include "Program.hpp"
#include "Clock.hpp"
#include "Config.hpp"
 

namespace map { namespace detail {

/*************
   Scheduler
 *************/

Scheduler::Scheduler(Clock &clock, Config &conf)
	: clock(clock)
	, conf(conf)
{ }

void Scheduler::clear() {
	job_vec_vec = decltype(job_vec_vec)();
	job_queue = decltype(job_queue)();
	job_set = decltype(job_set)();
	waiters_job = 0;
	end = false;
}

void Scheduler::initialJobs(const Program &prog) {
	TimedRegion region(clock,ADD_JOB);
	std::vector<Job> job_vec;

	assert(prog.taskList().size() > 0);
	
	// Filling 'job_vec' with all jobs of those tasks w/o prev dependencies
	for (auto task : prog.taskList())
		if (task->prevList().empty())
			task->initialJobs(job_vec);

	// Constructs 'job_queue' and 'job_set' from 'job_vec'
	for (auto job : job_vec) {
		job_queue.push(job);
		job_set.insert(job);
	}

	// Allocates the thread_local 'job_vec'
	job_vec_vec.resize(conf.num_workers);
}

Job Scheduler::requestJob() {
	TimedRegion region(clock,GET_JOB);

	while (true) {
		std::unique_lock<std::mutex> lock(mtx); // thread-safe
		
		if (job_queue.empty()) {
			waitForJob(lock);
			if (end) {
				return Job();
			}
		} else {
			Job job = job_queue.top();
			job_queue.pop();
			job_set.erase(job);
			return job;
		}
	}
}

void Scheduler::returnJob(Job job) {
	TimedRegion region(clock,RET_JOB);
	// Prepares the 'job_vec' to be filled with new 'jobs'
	std::vector<Job> &job_vec = job_vec_vec[Tid.proj()];
	job_vec.clear();
	 // Notifies that 'job' has finished and asks for its next-jobs
	job.task->askJobs(job,job_vec);
	 // Adds the next-jobs given in 'job_vec' by the notified taks
	addJobs(job_vec);
}

void Scheduler::waitForJob(std::unique_lock<std::mutex> &lock) {
	//TimedRegion region(clock,WAIT_JOB);

	waiters_job++;
	if (waiters_job == conf.num_workers) {
		end = true;  // Last waiter activates exit
		cv_job.notify_all();
	} else {
		cv_job.wait(lock);
	}
	waiters_job--;
}

void Scheduler::addJobs(const std::vector<Job> &job_vec) {
	std::lock_guard<std::mutex> lock(mtx); // thread-safe
	for (auto job : job_vec) {
		assert(job_set.find(job) == job_set.end());
		// Checks uniqueness before inserting
		//if (job_set.find(job) == job_set.end()) {
			job_queue.push(job);
			job_set.insert(job);
		//}
	}
	cv_job.notify_all();
}

void Scheduler::print() {
	std::cout << "--------------------" << std::endl;
	for (auto job : job_set)
		std::cout << job.task->id() << " " << job.coord << " " << job.iter << std::endl;
	std::cout << "--------------------" << std::endl;
}

} } // namespace map::detail
