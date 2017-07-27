/**
 * @file    Worker.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * NOTE: there is one worker per physical thread. Tid = per thread local storage for the ID = {node,device,rank}
 */

#ifndef MAP_RUNTIME_WORKER_HPP_
#define MAP_RUNTIME_WORKER_HPP_

#include "Job.hpp"
#include "block/BlockList.hpp"
#include "ThreadId.hpp"
#include "Config.hpp"
#include <vector>


namespace map { namespace detail {

class Cache; // Forward declaration
class Scheduler; // Forward declaration
class Clock; // Forward declaration

/*
 *
 */
class Worker
{
  public:
	Worker(Cache &cache, Scheduler &sche, Clock &clock, Config &conf);
	~Worker() = default;
	Worker(const Worker&) = delete;
	Worker& operator=(const Worker&) = delete;
	Worker(Worker&&) = default;
	Worker& operator=(Worker&&) = default;

	void work(ThreadId thread_id);

	void before_work();
	void request_blocks(Job job);
	void pre_load(Job job);
	void request_entries(Job job);
	void evict(Job job);
	void load(Job job);
	void pre_compute(Job job);
	void compute(Job job);
	void post_compute(Job job);
	void store(Job job);
	void post_store(Job job);
	void post_work(Job job);
	void return_entries(Job job);
	void return_blocks(Job job);
	void after_work();

  private:
	Cache &cache; // Aggregate
	Scheduler &sche; // Aggregate
	Clock &clock; // Aggregate
	Config &conf; // Aggregate

	KeyList in_key, out_key;
	BlockList in_blk, out_blk;
};

} } // namespace map::detail

#endif
