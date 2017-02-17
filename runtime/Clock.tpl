/**
 * @file	Clock.tpl 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_RUNTIME_CLOCK_TPL_
#define MAP_RUNTIME_CLOCK_TPL_


namespace map { namespace detail {

template <typename T>
T Clock::helper1(int enu, ThreadId id, std::function<T(TimerCounter &, int)> fn) {
	assert(id.mch() != ID_ALL && id.dev() != ID_ALL && id.rnk() != ID_ALL); // ID_ALL not allowed
	assert(id.mch() != ID_NONE || id.dev() == ID_NONE); // nod=none -> dev=none
	assert(id.dev() != ID_NONE || id.rnk() == ID_NONE); // dev=none -> rnk=none

	if (id.mch() == ID_NONE)
		return fn(system,enu);
	else if (id.dev() == ID_NONE)
		return fn(machine[id.mch()],enu);
	else if (id.rnk() == ID_NONE)
		return fn(device[id.mch()][id.dev()],enu);
	else
		return fn(rank[id.mch()][id.dev()][id.rnk()],enu);
}

} } // namespace map::detail

#endif
