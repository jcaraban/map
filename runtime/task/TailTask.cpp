/**
 * @file    TailTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: iter=0 wont work for nested loops. Needs to use the iter of its twin HeadTask!
 */

#include "TailTask.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

TailTask::TailTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{ }

void TailTask::blocksToStore(Job job, KeyList &out_key) const {
	job.iter = 0;
	Task::blocksToStore(job,out_key);
}

void TailTask::askJobs(Job done_job, std::vector<Job> &job_vec) {
	done_job.iter = 0;
	Task::askJobs(done_job,job_vec);
}

} } // namespace map::detail
