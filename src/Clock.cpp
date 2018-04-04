/**
 * @file	Clock.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Clock.hpp"
#include <cassert>
using namespace std::chrono;


namespace map { namespace detail {

Clock::Clock(Config &conf)
	: conf(conf)
{
	resize(); // Allocates
}

void Clock::resize() {
	machine.resize(conf.num_machines);
	device.resize(conf.num_machines);
	rank.resize(conf.num_machines);

	for (int m=0; m<conf.num_machines; m++) {
		device[m].resize(conf.num_devices);
		rank[m].resize(conf.num_devices);

		for (int d=0; d<conf.num_devices; d++)
			rank[m][d].resize(conf.num_ranks);
	}
}

void Clock::prepare() {
	ThreadId all = {ID_ALL,ID_ALL,ID_ALL};
	ThreadId sys = {ID_NONE,ID_NONE,ID_NONE};

	resize();

	for (int i=EVAL; i<N_TIMER; i++) {
		reset((TimerEnum)i,all);
		reset((TimerEnum)i,sys);
	}
	for (int i=LOADED; i<N_COUNTER; i++) {
		reset((CounterEnum)i,all);
		reset((CounterEnum)i,sys);
	}
}

void Clock::start(TimerEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &tc, int enu) {
		tc.timer_list[enu].point = system_clock::now();
	};
	helper1<void>(enu,id,fn);
}

void Clock::stop(TimerEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &tc, int enu) {
		auto dur = duration_cast<Duration>(system_clock::now() - tc.timer_list[enu].point);
		tc.timer_list[enu].duration += dur;
	};
	helper1<void>(enu,id,fn);
}

void Clock::incr(CounterEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &tc, int enu) {
		tc.counter_list[enu].counter++;
	};
	helper1<void>(enu,id,fn);
}

void Clock::decr(CounterEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &tc, int enu) {
		tc.counter_list[enu].counter--;
	};
	helper1<void>(enu,id,fn);
}

double Clock::get(TimerEnum enu, ThreadId id) {
	auto fn = [](const TimerCounter &tc, int enu) -> double {
		return tc.timer_list[enu].duration.count();
	};
	return helper1<double>(enu,id,fn);
}

size_t Clock::get(CounterEnum enu, ThreadId id) {
	auto fn = [](const TimerCounter &tc, int enu) -> size_t {
		return tc.counter_list[enu].counter;
	};
	return helper1<size_t>(enu,id,fn);
}

void Clock::helper2(int enu, ThreadId id, std::function<void(TimerCounter&, int)> fn) {
	assert(id.mch() != ID_ALL || id.dev() == ID_ALL || id.dev() == ID_NONE); // nod=all -> dev=all|none
	assert(id.dev() != ID_ALL || id.rnk() == ID_ALL || id.rnk() == ID_NONE); // dev=all -> rnk=all|none
	assert(id.mch() != ID_NONE || id.dev() == ID_NONE); // nod=none -> dev=none
	assert(id.dev() != ID_NONE || id.rnk() == ID_NONE); // dev=none -> rnk=none
	int m, M, d, D, r, R;
	
	M = (id.mch() == ID_NONE) ? 0 : (id.mch() == ID_ALL) ? conf.num_machines : id.mch()+1;
	D = (id.dev() == ID_NONE) ? 0 : (id.dev() == ID_ALL) ? conf.num_devices : id.mch()+1;
	R = (id.rnk() == ID_NONE) ? 0 : (id.rnk() == ID_ALL) ? conf.num_ranks : id.mch()+1;
	m = (id.mch() == ID_ALL) ? 0 : (id.mch() == ID_NONE) ? R : id.mch();
	d = (id.dev() == ID_ALL) ? 0 : (id.dev() == ID_NONE) ? D : id.dev();
	r = (id.rnk() == ID_ALL) ? 0 : (id.rnk() == ID_NONE) ? R : id.rnk();

	if (id.mch() == ID_NONE && id.dev() == ID_NONE && id.rnk() == ID_NONE)
		fn(system,enu);

	for (m; m<M; m++) {
		fn(machine[m],enu);
		for (d; d<D; d++) {
			fn(device[m][d],enu);
			for (r; r<R; r++) {
				fn(rank[m][d][r],enu);
			}
		}
	}
}

void Clock::reset(TimerEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &tc, int enu) {
		tc.timer_list[enu].duration = Duration(0);
	};
	helper2(enu,id,fn);
}

void Clock::reset(CounterEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &tc, int enu) {
		tc.counter_list[enu].counter = 0;
	};
	helper2(enu,id,fn);
}

void Clock::helper3(int enu, ThreadId id, std::function<void(TimerCounter&, TimerCounter&, int)> fn) {
	assert(id.mch() == ID_ALL || id.dev() == ID_ALL || id.rnk() == ID_ALL); // at least one ID_ALL
	assert(id.mch() != ID_ALL || id.dev() == ID_ALL || id.dev() == ID_NONE); // nod=all -> dev=all|none
	assert(id.dev() != ID_ALL || id.rnk() == ID_ALL || id.rnk() == ID_NONE); // dev=all -> rnk=all|none
	assert(id.mch() != ID_NONE); // nod != none
	assert(id.dev() != ID_NONE || id.rnk() == ID_NONE); // dev=none -> rnk=none
	int m, M, d, D, r, R;
	
	M = (id.mch() == ID_NONE) ? 0 : (id.mch() == ID_ALL) ? conf.num_machines : id.mch()+1;
	D = (id.dev() == ID_NONE) ? 0 : (id.dev() == ID_ALL) ? conf.num_devices : id.mch()+1;
	R = (id.rnk() == ID_NONE) ? 0 : (id.rnk() == ID_ALL) ? conf.num_ranks : id.mch()+1;
	m = (id.mch() == ID_ALL) ? 0 : (id.mch() == ID_NONE) ? R : id.mch();
	d = (id.dev() == ID_ALL) ? 0 : (id.dev() == ID_NONE) ? D : id.dev();
	r = (id.rnk() == ID_ALL) ? 0 : (id.rnk() == ID_NONE) ? R : id.rnk();

	for (m; m<M; m++) {
		for (d; d<D; d++) {
			for (r; r<R; r++) {
				if (id.rnk() == ID_ALL)
					fn(device[m][d],rank[m][d][r],enu);
			}
			if (id.dev() == ID_ALL)
				fn(machine[m],device[m][d],enu);
		}
		if (id.mch() == ID_ALL)
			fn(system,machine[m],enu);
	}
}

void Clock::sync(TimerEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &above, TimerCounter &below, int enu) {
		above.timer_list[enu].duration += below.timer_list[enu].duration;
	};
	helper3(enu,id,fn);
}

void Clock::sync(CounterEnum enu, ThreadId id) {
	auto fn = [](TimerCounter &above, TimerCounter &below, int enu) {
		above.counter_list[enu].counter += below.counter_list[enu].counter;
	};
	helper3(enu,id,fn);
}

void Clock::syncAll(ThreadId id) {
	assert(id.mch() == ID_ALL || id.dev() == ID_ALL || id.rnk() == ID_ALL); // at least one ID_ALL
	assert(id.mch() != ID_ALL || id.dev() == ID_ALL || id.dev() == ID_NONE); // nod=all -> dev=all|none
	assert(id.dev() != ID_ALL || id.rnk() == ID_ALL || id.rnk() == ID_NONE); // dev=all -> rnk=all|none
	assert(id.mch() != ID_NONE); // nod != none
	assert(id.dev() != ID_NONE || id.rnk() == ID_NONE); // dev=none -> rnk=none

	// TODO: are the 3 branches necessary?

	if (id.mch() == ID_ALL && id.dev() == ID_ALL && id.rnk() == ID_ALL)
	{
		for (int i=NONE_TIMER+1; i<N_TIMER; i++) {
			sync(static_cast<TimerEnum>(i),{id.mch(),id.dev(),id.rnk()});
		}
		for (int i=NONE_COUNTER+1; i<N_COUNTER; i++) {
			sync(static_cast<CounterEnum>(i),{id.mch(),id.dev(),id.rnk()});
		}
	}
	else if (id.mch() == ID_ALL && id.dev() == ID_ALL)
	{
		for (int i=NONE_TIMER+1; i<N_TIMER; i++) {
			sync(static_cast<TimerEnum>(i),{id.mch(),id.dev(),ID_NONE});
		}
		for (int i=NONE_COUNTER+1; i<N_COUNTER; i++) {
			sync(static_cast<CounterEnum>(i),{id.mch(),id.dev(),ID_NONE});
		}
	}
	else if (id.mch() == ID_ALL)
	{
		for (int i=NONE_TIMER+1; i<N_TIMER; i++) {
			sync(static_cast<TimerEnum>(i),{id.mch(),ID_NONE,ID_NONE});
		}
		for (int i=NONE_COUNTER+1; i<N_COUNTER; i++) {
			sync(static_cast<CounterEnum>(i),{id.mch(),ID_NONE,ID_NONE});
		}
	}
	else
	{
		assert(0);
	}
}

} } // namespace map::detail
