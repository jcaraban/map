/**
 * @file    Job.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: prev-tasks to a radiating one  w/o strict block ordering should inherit the radiating sheduling
 * TODO: The optimal ordering (scheduling) of jobs is an NP-hard problem, how to deal with it?
 */

#include "Job.hpp"
#include "task/Task.hpp"
#include "task/RadiatingTask.hpp" // @
#include "task/SpreadingTask.hpp" // @
#include "Runtime.hpp"
#include <functional>


namespace map { namespace detail {

Order::Order()
	: order{0,0,0,0}
{ }

Order::Order(uint x, uint y, uint t, uint h)
	: order{0,0,0,0}
{
	if (Runtime::getConfig().inmem_cache == false) {
		order[0]=h; order[1]=t; order[2]=y; order[3]=x;
		return; // return with normal {h,t,y,x} order
	}

	const uint S = sizeof(int)*8;

	for (uint i=0; i<S*N; i++) {
		uint m = i % N;
		uint p = N-1 - i / S;
		uint b = i / N;
		/****/ if (m == 0) {
			order[p] |= (h & (1 << b)) << (N*b+0);
		} else if (m == 1) {
			order[p] |= (t & (1 << b)) << (N*b+1);
		} else if (m == 2) {
			order[p] |= (y & (1 << b)) << (N*b+2);
		} else if (m == 3) {
			order[p] |= (x & (1 << b)) << (N*b+3);
		}
	}
}

bool Order::operator<(const Order &other) const {
	for (int i=0; i<N; i++) {
		if (order[i] < other.order[i])
			return true;
		else if (order[i] > other.order[i])
			return false;
	}
	return false;
}

Job::Job()
	: task(nullptr)
	, coord{-1,-1} // @
	, order()
{ }

Job::Job(Task *task, Coord coord)
	: task(task)
	, coord(coord)
	, order()
{
	Coord dif;
	auto *rt = dynamic_cast<RadiatingTask*>(task); // @
	if (rt != nullptr) {
		dif = abs(coord - rt->startb);
	} else {
		dif = coord;
	}

	uint h;
	auto *st = dynamic_cast<SpreadingTask*>(task); // @
	if (st != nullptr) {
		h = st->height(coord);
	} else {
		h = 0;
	}

	order = Order(dif[0],dif[1],task->id(),h);
}

bool job_cmp::operator() (const Job &lhs, const Job &rhs) const {
	return rhs.order < lhs.order;
}

std::size_t job_hash::operator()(const Job& j) const {
	/*std::size_t h = std::hash<Task*>()(j.task);
	for (int i=0; i<j.coord.size(); i++)
		h ^= std::hash<int>()(j.coord[i]);*/
	std::size_t h = (size_t)j.task & 0x00000000ffffffff;
	for (int i=0; i<j.coord.size(); i++)
		h |= (size_t)j.coord[i] << (32+i*16);
	return h;
}

} } // namespace map::detail
