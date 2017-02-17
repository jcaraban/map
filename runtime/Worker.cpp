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
		Job job = sche.getJob();
		
		if (job.task == nullptr) break; // Exit point
			//std::cout << job.task->id() << job.coord << std::endl;
		load(job);

		compute(job);
		
		store(job);
		
		sche.notifyEnd(job);
	}
}

void Worker::load(Job job) {
	TimedRegion region(clock,LOAD); // Timed function

	job.task->preLoad(job.coord);

	job.task->blocksToLoad(job.coord,in_keys);
	cache.retainInputBlocks(in_keys,in_blk);

	job.task->blocksToStore(job.coord,out_keys);
	cache.retainOutputBlocks(out_keys,out_blk);
}

void Worker::store(Job job) {
	TimedRegion region(clock,STORE); // Timed function

	cache.releaseInputBlocks(in_blk);
	cache.releaseOutputBlocks(out_blk,out_keys);

	job.task->postStore(job.coord);
}

void Worker::compute(Job job) {
	TimedRegion region(clock,COMPUTE); // Timed function

	 // @ tries to predict the result according to some fixed inputs
	Predictor predictor(job.task->base_group);
	if (predictor.predict(job.coord,in_blk,out_blk)) {
			//std::cout << job.task->id() << job.coord << std::endl;
		clock.incr(NOT_COMPUTED);
		return;
	} else {
		clock.incr(COMPUTED);
	}

	job.task->preCompute(job.coord,in_blk,out_blk);
	job.task->compute(job.coord,in_blk,out_blk);
	job.task->postCompute(job.coord,in_blk,out_blk);
}

} } // namespace map::detail
