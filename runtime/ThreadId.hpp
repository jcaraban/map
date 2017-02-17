/**
 * @file	ThreadId.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_THREADID_HPP_
#define MAP_RUNTIME_THREADID_HPP_

#include "Config.hpp"


namespace map { namespace detail {

enum IdEnum : signed {ID_ALL=-1, ID_NONE=-2};

/*
 *
 */
class ThreadId {
	int machine; //!< Machine in the distrubuted network
	int device; //!< Device in the OpenCL framework
	int rank; //!< Rank of the worker

  public:
	ThreadId() : machine(ID_NONE), device(ID_NONE), rank(ID_NONE) { }
	ThreadId(int machine, int device, int rank) : machine(machine), device(device), rank(rank) { }
	bool operator==(ThreadId &other);
	int mch() { return machine; }
	int dev() { return device; }
	int rnk() { return rank; }
	int proj(); // Projectios of nod+dev+rnk index in 1D
};

extern thread_local ThreadId Tid; //!< Per thread local storage for the ID = {machine,device,rank}

} } // namespace map::detail

#endif
