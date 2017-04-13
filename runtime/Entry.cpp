/**
 * @file	Entry.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Entry.hpp"
#include <functional>


namespace map { namespace detail {

Entry::Entry(cl_mem dev_mem)
	: dev_mem(dev_mem)
	, host_mem(nullptr)
	, block(nullptr)
	, used(0)
	, dirty(false)
{ }

void Entry::reset() {
	host_mem = nullptr;
	block = nullptr;
	used = 0;
	dirty = false;
}

void Entry::setDirty() {
	assert(not dirty);
	dirty = true;
}

void Entry::unsetDirty() {
	assert(dirty);
	dirty = false;	
}

bool Entry::isDirty() {
	return dirty;
}

void Entry::setUsed() {
	used++;
}

void Entry::unsetUsed() {
	assert(used > 0);
	used--;
}

bool Entry::isUsed() {
	return used > 0;
}

} } // namespace map::detail
