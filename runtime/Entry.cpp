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
	, loading(false)
	, writing(false)
{ }

void Entry::setDirty() {
	dirty = true;
}

void Entry::unsetDirty() {
	dirty = false;	
}

bool Entry::isDirty() {
	return dirty;
}

void Entry::setUsed() {
	used++;
}

void Entry::unsetUsed() {
	used--;
}

bool Entry::isUsed() {
	return used > 0;
}

void Entry::setLoading() {
	loading = true;
}

void Entry::unsetLoading() {
	loading = false;
}

bool Entry::isLoading() {
	return loading;
}

void Entry::setWriting() {
	writing = true;
}

void Entry::unsetWriting() {
	writing = false;
}

bool Entry::isWriting() {
	return writing;
}

} } // namespace map::detail
