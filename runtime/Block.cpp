/**
 * @file	Block.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: reduce constructors to just one ?
 */

#include "Block.hpp"
#include "Runtime.hpp"
#include <algorithm>


namespace map { namespace detail {

/*********
   Stats
 *********/

BlockStats::BlockStats()
	: active(false)
	, max()
	, mean()
	, min()
	, std()
	, ming()
	, maxg()
	, meang()
	, stdg()
{ }

/*********
   Block
 *********/

Block::Block()
	: key()
	, entry(nullptr)
	, host_mem(nullptr)
	, scalar_page(nullptr)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(-1)
	, dependencies(DEPEND_UNKNOWN)
	, hold_type()
	, used(0)
	, dirty(false)
	, order(-1)
	, mtx()
{ }

Block::Block(Key key, int dep)
	: key(key)
	, entry(nullptr)
	, host_mem(nullptr)
	, scalar_page(nullptr)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(-1)
	, dependencies(dep)
	, hold_type(HOLD_0)
	, used(0)
	, dirty(false)
	, order(-1)
	, mtx()
{ }

Block::Block(Key key, int dep, cl_mem scalar_page, cl_mem group_page)
	: key(key)
	, entry(nullptr)
	, host_mem(nullptr)
	, scalar_page(scalar_page)
	, group_page(nullptr)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(-1)
	, dependencies(dep)
	, hold_type(HOLD_1)
	, used(0)
	, dirty(false)
	, order(-1)
	, mtx()
{ }
/*
Block::Block(Key key, int dep, cl_mem group_page, int size)
	: key(key)
	, entry(nullptr)
	, host_mem(nullptr)
	, scalar_page(nullptr)
	, group_page(group_page)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(size)
	, dependencies(dep)
	, hold_type(HOLD_2)
	, used(0)
	, dirty(false)
	, order(-1)
	, mtx()
{ }
*/
Block::Block(Key key, int dep, int max_size)
	: key(key)
	, entry(nullptr)
	, host_mem(nullptr)
	, scalar_page(nullptr)
	, group_page(nullptr)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(-1)
	, dependencies(dep)
	, hold_type(HOLD_N)
	, used(0)
	, dirty(false)
	, order(-1)
	, mtx()
{
	this->total_size = prod(key.node->blocksize()) * datatype().sizeOf();
	assert(total_size <= max_size);
	assert(max_size % total_size == 0);
}

Block::~Block() { }

int Block::size() const {
	return total_size;
}

StreamDir Block::streamdir() const {
	return key.node->streamdir();
}

DataType Block::datatype() const {
	return key.node->datatype();
}

NumDim Block::numdim() const {
	return key.node->numdim();
}

MemOrder Block::memorder() const {
	return key.node->memorder();
}

HoldType Block::holdtype() const {
	return hold_type;
}

Berr Block::send() {
	TimedRegion region(Runtime::getClock(),SEND);
	cle::Queue que = Runtime::getOclEnv().D(Tid.dev()).Q(Tid.rnk());
	Berr berr = 0;
	cl_int clerr;

	if (!fixed) {
		clerr = clEnqueueWriteBuffer(*que, entry->dev_mem, CL_TRUE, 0, size(), host_mem, 0, nullptr, nullptr);
		cle::clCheckError(clerr);
	}

	return berr;
}

Berr Block::recv() {
	TimedRegion region(Runtime::getClock(),RECV);
	cle::Queue que = Runtime::getOclEnv().D(Tid.dev()).Q(Tid.rnk());
	Berr berr = 0;
	cl_int clerr;

	if (!fixed) {
		clerr = clEnqueueReadBuffer(*que, entry->dev_mem, CL_TRUE, 0, size(), host_mem, 0, nullptr, nullptr);
		cle::clCheckError(clerr);
	} else {
		const size_t num = prod(key.node->blocksize());
		value.fill(host_mem,num); // @
	}

	return berr;
}

Berr Block::load(IFile *file) {
	TimedRegion region(Runtime::getClock(),READ);
	StreamDir dir = key.node->streamdir();
	NumDim dim = key.node->numdim();

	assert(dir == IN || dir == IO);
	//assert(dim != D0 || scalar_page != nullptr); //D0 --> scalar_page
	//assert(dim == D0 || entry->dev_mem != nullptr); // !D0 --> entry
	//assert(dim == D0 || host_mem != nullptr); // !D0 --> entry

	Ferr ferr = file->readBlock(*this);

	assert(ferr == 0); // any check?
	Berr berr = ferr; // from ferr to berr

	return berr;
}

Berr Block::store(IFile *file) {
	TimedRegion region(Runtime::getClock(),WRITE);
	StreamDir dir = key.node->streamdir();
	NumDim dim = key.node->numdim();

	assert(dir == OUT || dir == IO);
	//assert(dim != D0 || scalar_page != nullptr); // D0 --> scalar_page
	//assert(dim == D0 || entry->dev_mem != nullptr); // !D0 --> entry
	//assert(dim == D0 || host_mem != nullptr); // !D0 --> entry

	Ferr ferr = file->writeBlock(*this);

	assert(ferr == 0); // any check?
	Berr berr = ferr; // from ferr to berr

	return berr;
}

void Block::fixValue(ValFix vf) {

	fixed = vf.fixed;
	if (fixed) {
		ready = true;
		value = vf.value;
		stats.active = true;
		stats.max = vf.max.get();
		stats.min = vf.min.get();
		stats.mean = vf.mean.get();
		stats.std = vf.std.get();
	}
}

void Block::notify() {
	assert(dependencies > 0); // @@ before there was an "if (dep >= 0)"
	dependencies--;
}

bool Block::discardable() const {
	return dependencies == DEPEND_ZERO; // == 0
}

void Block::setReady() {
	assert(not ready);
	ready = true;
}

void Block::unsetReady() {
	assert(ready);
	ready = false;
}

bool Block::isReady() {
	return ready;
}

void Block::setDirty() {
	assert(not dirty);
	dirty = true;
	if (entry != nullptr)
		entry->setDirty();
}

void Block::unsetDirty() {
	assert(dirty);
	dirty = false;
	if (entry != nullptr)
		entry->unsetDirty();	
}

bool Block::isDirty() {
	if (entry != nullptr)
		assert(dirty == entry->dirty);
	return dirty;
}

void Block::setUsed() {
	used++;
	if (entry != nullptr)
		entry->setUsed();
}

void Block::unsetUsed() {
	assert(used > 0);
	used--;
	if (entry != nullptr)
		entry->unsetUsed();
}

bool Block::isUsed() {
	if (entry != nullptr)
		assert(used == entry->used);
	return used > 0;
}

} } // namespace map::detail
