/**
 * @file    Scheduler.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Scheduler.hpp"
#include "Program.hpp"
#include "Clock.hpp"
 

namespace map { namespace detail {

/*************
   Scheduler
 *************/

Scheduler::Scheduler(Program &prog, Clock &clock, Config &conf)
	: prog(prog)
	, clock(clock)
	, conf(conf)
{ }

void Scheduler::clear() {
	job_vec_vec = decltype(job_vec_vec)();
	job_queue = decltype(job_queue)();
	job_set = decltype(job_set)();
	waiters_job = 0;
	end = false;
}

void Scheduler::initialJobs() {
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

	// Allocates the 'job_vec' for the threads
	job_vec_vec.resize( conf.num_machines*conf.num_devices*conf.num_ranks );
}

Job Scheduler::requestJob() {
	TimedRegion region(clock,GET_JOB);
	Job job;

	while (true) {
		std::unique_lock<std::mutex> lock(mtx); // thread-safe
		
		if (job_queue.empty()) {
			waitForJob(lock);
			if (end) {
				job.task = nullptr;
				return job;
			}
		} else {
			job = job_queue.top();
			job_queue.pop();
			job_set.erase(job);
			return job;
		}
	}
}

void Scheduler::returnJob(Job job) {
	TimedRegion region(clock,NOTIFY);
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
		// Checks uniqueness before inserting
		if (job_set.find(job) == job_set.end()) {
			job_queue.push(job);
			job_set.insert(job);
		}
	}
	cv_job.notify_all();
}

void Scheduler::print() {
	// print queues here?
}

} } // namespace map::detail
