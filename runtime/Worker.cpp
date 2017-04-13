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
	in_keys.reserve(conf.max_in_block);
	in_blk.reserve(conf.max_in_block);
	out_keys.reserve(conf.max_in_block);
	out_blk.reserve(conf.max_in_block);
}

void Worker::work(ThreadId thread_id) {
	Tid = thread_id; // Local thread id initialization
	
	// @@ SymLoop 'condition' + 'unrolling' needs to happen at local level
	// the worker needs to walk the nodes and make more decisions
	// all this without killing the performance, i.e. will be challenging

	// Shall the worker unroll the loop for himself if it is LOCAL?

	while (true) // Worker loop
	{
		Job job = sche.requestJob();
		
		if (job.task == nullptr) break; // Exit point

		get_blocks(job);

		pre_load(job);

		load(job);

		//pre_comp(job);

		compute(job);

		//post_comp(job)

		store(job);

		post_store(job);

		//return_blocks ?

		sche.returnJob(job);
	}
}

void Worker::get_blocks(Job job) {
	TimedRegion region(clock,GET_BLOCK); // Timed function

	job.task->blocksToLoad(job.coord,in_keys);
	job.task->blocksToStore(job.coord,out_keys);

	cache.requestInputBlocks(in_keys,in_blk);
	cache.requestOutputBlocks(out_keys,out_blk);
}

void Worker::pre_load(Job job) {
	TimedRegion region(clock,PRE_LOAD); // Timed function

	job.task->preLoad(job.coord);

	Predictor predictor(job.task->base_group);
	predictor.predict(job.coord,in_blk,out_blk);
}

void Worker::load(Job job) {
	TimedRegion region(clock,LOAD); // Timed function

	cache.retainInputEntries(in_blk);
	cache.retainOutputEntries(out_blk);
	// ! those outputs that were 'fixed' by predictor don't need entry anymore

	cache.readInputBlocks(in_blk);
}

void Worker::compute(Job job) {
	TimedRegion region(clock,COMPUTE); // Timed function

	job.task->preCompute(job.coord,in_blk,out_blk);
	job.task->compute(job.coord,in_blk,out_blk);
	job.task->postCompute(job.coord,in_blk,out_blk);
}

void Worker::store(Job job) {
	TimedRegion region(clock,STORE); // Timed function

	cache.writeOutputBlocks(out_blk);

	cache.returnInputBlocks(in_blk);
	cache.returnOutputBlocks(out_blk);
}

void Worker::post_store(Job job) {
	TimedRegion region(clock,POST_STORE); // Timed function

	job.task->postStore(job.coord);
}

} } // namespace map::detail
