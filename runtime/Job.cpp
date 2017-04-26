/**
 * @file    Job.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: prev-tasks to a Radial one  w/o strict block ordering should inherit the Radial sheduling
 */

#include "Job.hpp"
#include "task/Task.hpp"
#include "task/RadialTask.hpp" // @
#include "Runtime.hpp"
#include <functional>
#include <iostream>


namespace map { namespace detail {

Order::Order()
	: order{0,0,0,0}
{ }

Order::Order(uint x, uint y, uint z, uint t)
	: order{0,0,0,0}
{
	if (Runtime::getConfig().inmem_cache == false) {
		order[0]=t; order[1]=z; order[2]=y; order[3]=x;
		return; // return with normal {h,t,y,x} order
	}

	const uint L = sizeof(int)*8; // 32 bits per int

	for (uint i=0; i<L*N; i++) { // 128 bits to fill
		uint m = i % N; // 0,1,2,3,0,1,2,3...
		uint p = N-1 - i / L; // 3,3...2,2...1,1...0,0...
		uint b = i / N; // 0,0,0,0,1,1,1,1,2,2,2,2,...
		uint s = (static_cast<uint>(1) << b); // shift
		/****/ if (m == 0) {
			order[p] |= (x & s) << (N*b+0);
		} else if (m == 1) {
			order[p] |= (y & s) << (N*b+1);
		} else if (m == 2) {
			order[p] |= (z & s) << (N*b+2);
		} else if (m == 3) {
			order[p] |= (t & s) << (N*b+3);
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
	, coord()
	, order()
{ }

Job::Job(Task *task, Coord coord)
	: task(task)
	, coord(coord)
	, order()
{
	Coord dif;
	auto *rt = dynamic_cast<RadialTask*>(task); // @
	if (rt != nullptr) {
		dif = abs(coord - rt->startb);
	} else {
		dif = coord;
	}

	order = Order(dif[0],dif[1],task->id(),0);
}

bool job_greater::operator() (const Job &lhs, const Job &rhs) const {
	return rhs.order < lhs.order;
}

bool job_equal::operator() (const Job &lhs, const Job &rhs) const {
	return lhs.task == rhs.task && all(lhs.coord == rhs.coord);
}

std::size_t job_hash::operator()(const Job& j) const {
	std::size_t h = std::hash<Task*>()(j.task);
	for (int i=0; i<j.coord.size(); i++)
		h ^= std::hash<int>()(j.coord[i]);
	return h;
}

bool Job::isNone() {
	return task == nullptr;
}

} } // namespace map::detail
