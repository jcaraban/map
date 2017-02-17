/**
 * @file    Job.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: jobs are allocated in the stack now (pqueue). It might be better to allocate Jobs in the heap
 *       It might be good to add Jobs a pointer to their neighbors when they are added to the queue
 *       (think carefully... premature optimization and so on)
 */

#ifndef MAP_RUNTIME_JOB_HPP_
#define MAP_RUNTIME_JOB_HPP_

#include "../util/Array.hpp"
#include "../util/Array4.hpp"


namespace map { namespace detail {

typedef unsigned int uint;
struct Task; // Forward declaration

/*
 * Job order, as computed by a Space-Filling Curve
 * Current SFC = Z-order / Morton encoding
 */
struct Order {
	static const int N = 4;
	unsigned int order[N];

	Order();
	Order(uint x, uint y, uint t, uint h);
	bool operator<(const Order &other) const;
};

/*
 * @class Job
 * Execution and scheduling unit. Composed of a pair {task,block}
 */
struct Job {
	Task *task;
	Coord coord;
	Order order;

	Job();
	Job(Task *task, Coord coord);
};

struct job_cmp {
	bool operator()(const Job &lhs, const Job &rhs) const;
};

struct job_hash {
	std::size_t operator()(const Job &j) const;
};

} } // namespace map::detail

#endif
