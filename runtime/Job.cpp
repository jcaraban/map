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
		order[0]=x; order[1]=y; order[2]=z; order[3]=t;
		return; // return with normal {x,y,z,t} order
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
	, iter(0)
	, order()
{ }

Job::Job(Task *task, Coord coord, size_t iter)
	: task(task)
	, coord(coord)
	, iter(iter)
	, order()
{
	auto diff = coord;
	auto *rt = dynamic_cast<RadialTask*>(task); // @@
	if (rt != nullptr) {
		diff = abs(coord - rt->startb);
	}

	order = Order(diff[0],diff[1],task->id(),iter);
}

bool job_greater::operator() (const Job &lhs, const Job &rhs) const {
	return rhs.order < lhs.order;
}

bool job_equal::operator() (const Job &lhs, const Job &rhs) const {
	return lhs.task == rhs.task && all(lhs.coord == rhs.coord) && lhs.iter==rhs.iter;
}

std::size_t job_hash::operator()(const Job& j) const {
	return std::hash<Task*>()(j.task) ^ coord_hash()(j.coord) ^ std::hash<int>()(j.iter);
}

bool Job::isNone() {
	return task == nullptr;
}

} } // namespace map::detail
