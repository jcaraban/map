/**
 * @file    Worker.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: the prediction should happens before any 'data block' is read from disk, only with the 'data stats'
 * TODO: for more dynamism, worker should analyse / fuse / compile online ('runtime approach to map algebra')
 */

#include "Worker.hpp"
#include "Cache.hpp"
#include "Scheduler.hpp"
#include "Clock.hpp"
#include "task/Task.hpp"
#include "visitor/Predictor.hpp"


namespace map { namespace detail {

/**********
   Worker
 **********/

Worker::Worker(Cache &cache, Scheduler &sche, Clock &clock, Config &conf)
	: cache(cache)
	, sche(sche)
	, clock(clock)
	, conf(conf)
{
	in_key.reserve(conf.max_in_block);
	in_blk.reserve(conf.max_in_block);
	out_key.reserve(conf.max_in_block);
	out_blk.reserve(conf.max_in_block);
}

void Worker::work(ThreadId thread_id) {
	Tid = thread_id; // Local thread id initialization
	
	// @@ SymLoop 'condition' + 'unrolling' needs to happen locally

	while (true) // Worker loop
	{
		before_work();

		Job job = sche.requestJob();
		
		if (job.isNone()) break; // Exit point

		request_blocks(job);

		pre_load(job);

		load(job);

		pre_compute(job);

		compute(job);

		post_compute(job);

		store(job);

		post_store(job);

		return_blocks(job);

		sche.returnJob(job);

		after_work();
	}
}

void Worker::before_work() {
	// what could be do 'before work' and not related to the job
	// e.g. machine related stuff, debug checks,  distributed exchange (of version_cache, statistics) ?
}

void Worker::request_blocks(Job job) {
	TimedRegion region(clock,GET_BLOCK); // Timed function

	job.task->blocksToLoad(job.coord,in_key);
	job.task->blocksToStore(job.coord,out_key);

	cache.requestBlocks(in_key,in_blk);
	cache.requestBlocks(out_key,out_blk);
}

void Worker::pre_load(Job job) {
	TimedRegion region(clock,PRE_LOAD); // Timed function

	job.task->preLoad(job.coord,in_blk,out_blk);
}

void Worker::load(Job job) {
	TimedRegion region(clock,LOAD); // Timed function

	cache.retainEntries(in_blk);
	cache.retainEntries(out_blk);

	cache.readInputBlocks(in_blk);
}

void Worker::pre_compute(Job job) {
	TimedRegion region(clock,PRE_COMP); // Timed function

	// choose a version among the available, according to statistics, devices ?

	job.task->preCompute(job.coord,in_blk,out_blk);
}

void Worker::compute(Job job) {
	TimedRegion region(clock,COMPUTE); // Timed function

	job.task->compute(job.coord,in_blk,out_blk);
}

void Worker::post_compute(Job job) {
	TimedRegion region(clock,POST_COMP); // Timed function

	job.task->postCompute(job.coord,in_blk,out_blk);
}

void Worker::store(Job job) {
	TimedRegion region(clock,STORE); // Timed function

	cache.writeOutputBlocks(out_blk);

	cache.releaseEntries(in_blk);
	cache.releaseEntries(out_blk);
}

void Worker::post_store(Job job) {
	TimedRegion region(clock,POST_STORE); // Timed function

	job.task->postStore(job.coord,in_blk,out_blk);
}

void Worker::return_blocks(Job job) {
	TimedRegion region(clock,RET_BLOCK); // Timed function

	cache.returnBlocks(in_key,in_blk);
	cache.returnBlocks(out_key,out_blk);
}

void Worker::after_work() {
	// a place for statiscs based 'code transformations', 'adaptive refinement', etc?
	// e.g. trying different loop unrolling factors, different fusion rules
}

} } // namespace map::detail
