/**
 * @file	Clock.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Clock class, needs to be extremelly efficient because it is called on the critical loop
 */

#ifndef MAP_RUNTIME_CLOCK_HPP_
#define MAP_RUNTIME_CLOCK_HPP_

#include "Config.hpp"
#include "ThreadId.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>


namespace map { namespace detail {

// Enum

enum TimerEnum { NONE_TIMER, OVERALL, DEVICES, EVAL, ALLOC_C, FUSION, TASKIF, CODGEN, COMPIL, ADD_JOB, ALLOC_E, EXEC, FREE_E, FREE_C,
				 GET_JOB, GET_BLOCK, PRE_LOAD, LOAD, PRE_COMP, COMPUTE, POST_COMP, STORE, POST_STORE, RET_BLOCK, RET_JOB,
				 READ, SEND, KERNEL, RECV, WRITE, N_TIMER };

enum CounterEnum { NONE_COUNTER, LOADED, STORED, COMPUTED, DISCARDED, EVICTED, NOT_LOADED, NOT_STORED, NOT_COMPUTED, N_COUNTER };

/*
 *
 */
class Clock
{
	typedef std::chrono::duration<double> Duration;
	typedef std::chrono::time_point<std::chrono::system_clock> Timepoint;

	struct TimerStruct {
		Timepoint point;
		Duration duration;
		TimerStruct() : duration(0) { }
	};

	struct CounterStruct {
		size_t counter;
		CounterStruct() : counter(0) { }
	};

	typedef std::array<TimerStruct,N_TIMER> TimerList;
	typedef std::array<CounterStruct,N_COUNTER> CounterList;

	struct TimerCounter {
		TimerList timer_list;
		CounterList counter_list;
	};	

	template <typename T> T helper1(int enu, ThreadId id, std::function<T(TimerCounter&, int)> fn);
	void helper2(int enu, ThreadId id, std::function<void(TimerCounter&, int)> fn);
	void helper3(int enu, ThreadId id, std::function<void(TimerCounter&, TimerCounter&, int)> fn);

  public:
	Clock(Config &conf);
	void resize();
	void prepare();

	void start(TimerEnum enu, ThreadId id=Tid);
	void stop(TimerEnum enu, ThreadId id=Tid);
	double get(TimerEnum enu, ThreadId id=Tid);
	void reset(TimerEnum enu, ThreadId id);
	
	void incr(CounterEnum enu, ThreadId id=Tid);
	void decr(CounterEnum enu, ThreadId id=Tid);
	size_t get(CounterEnum enu, ThreadId id=Tid);
	void reset(CounterEnum enu, ThreadId id);

	void sync(TimerEnum enu, ThreadId id);
	void sync(CounterEnum enu, ThreadId id);
	void syncAll(ThreadId id);

	//int loaded, stored, computed, discarded, evicted;
	//int not_loaded, not_stored, not_computed; // @

  private:
  	Config &conf; // Aggregate
  	TimerCounter system;
  	std::vector<TimerCounter> machine;
  	std::vector<std::vector<TimerCounter>> device;
  	std::vector<std::vector<std::vector<TimerCounter>>> rank;
};


/*
 *
 */
class TimedRegion
{
	Clock &clock;
	TimerEnum enu;
  public:
  	TimedRegion(Clock &clock, TimerEnum enu) : clock(clock), enu(enu) { clock.start(enu); }
  	~TimedRegion() { clock.stop(enu); }
  	TimedRegion(const TimedRegion&) = delete;
  	TimedRegion& operator=(const TimedRegion&) = delete;
};

} } // namespace map::detail

#endif

#include "Clock.tpl"
