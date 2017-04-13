/**
 * @file	Entry.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: is 'host_mem' necessary?
 */

#ifndef MAP_RUNTIME_ENTRY_HPP_
#define MAP_RUNTIME_ENTRY_HPP_

#include "../cle/OclEnv.hpp"


namespace map { namespace detail {

struct Block; // forward declaration

struct Entry {	
  // Constructors & methods
	Entry(cl_mem dev_mem);

	void reset();
	void setDirty();
	void unsetDirty();
	bool isDirty();
	void setUsed();
	void unsetUsed();
	bool isUsed();

  // Variables
	std::list<Entry*>::iterator self;
	cl_mem dev_mem;
	void *host_mem;
	Block *block;
	char used;
	bool dirty;
};

} } // namespace map::detail

#endif
