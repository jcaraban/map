/**
 * @file    Worker.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * NOTE: there is one worker per physical thread. Tid = per thread local storage for the ID = {node,device,rank}
 *
 * TODO: statistics at block level can be used to speed up the execution (e.g. if max==min -> all values are same)
 */

#ifndef MAP_RUNTIME_WORKER_HPP_
#define MAP_RUNTIME_WORKER_HPP_

#include "Job.hpp"
#include "Entry.hpp"
#include "Block.hpp"
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
	void load(Job job);
	void store(Job job);
	void compute(Job job);

  private:
	Cache &cache; // Aggregate
	Scheduler &sche; // Aggregate
	Clock &clock; // Aggregate
	Config &conf; // Aggregate

	InKeyList in_keys;
	BlockList in_blk;
	OutKeyList out_keys;
	BlockList out_blk;
};

} } // namespace map::detail

#endif
