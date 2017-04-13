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
{ }

/*********
   Block
 *********/

Block::Block()
	: key()
	, entry(nullptr)
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
	, loading(false)
	, writing(false)
{ }

Block::Block(Key key)
	: key(key)
	, entry(nullptr)
	, scalar_page(nullptr)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(-1)
	, dependencies(DEPEND_UNKNOWN)
	, hold_type(HOLD_0)
	, used(0)
	, dirty(false)
	, loading(false)
	, writing(false)
{ }

Block::Block(Key key, cl_mem scalar_page)
	: key(key)
	, entry(nullptr)
	, scalar_page(scalar_page)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(-1)
	, dependencies(DEPEND_UNKNOWN)
	, hold_type(HOLD_1)
	, used(0)
	, dirty(false)
	, loading(false)
	, writing(false)
{
	assert(numdim() == D0);
}

Block::Block(Key key, int max_size, int depend)
	: key(key)
	, entry(nullptr)
	, scalar_page(nullptr)
	, value()
	, fixed(false)
	, ready(false)
	, stats()
	, total_size(-1)
	, dependencies(depend)
	, hold_type(HOLD_N)
	, used(0)
	, dirty(false)
	, loading(false)
	, writing(false)
{
	assert(numdim() != D0);
	
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
		clerr = clEnqueueWriteBuffer(*que, entry->dev_mem, CL_TRUE, 0, size(), entry->host_mem, 0, nullptr, nullptr);
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
		clerr = clEnqueueReadBuffer(*que, entry->dev_mem, CL_TRUE, 0, size(), entry->host_mem, 0, nullptr, nullptr);
		cle::clCheckError(clerr);
	} else {
		const int n = prod(key.node->blocksize());
		switch (datatype().get()) {
			case F32 : std::fill_n((Ctype<F32>*)entry->host_mem,n,value.get<F32>()); break;
			case F64 : std::fill_n((Ctype<F64>*)entry->host_mem,n,value.get<F64>()); break;
			case B8  : std::fill_n((Ctype<B8 >*)entry->host_mem,n,value.get<B8 >()); break;
			case U8  : std::fill_n((Ctype<U8 >*)entry->host_mem,n,value.get<U8 >()); break;
			case U16 : std::fill_n((Ctype<U16>*)entry->host_mem,n,value.get<U16>()); break;
			case U32 : std::fill_n((Ctype<U32>*)entry->host_mem,n,value.get<U32>()); break;
			case U64 : std::fill_n((Ctype<U64>*)entry->host_mem,n,value.get<U64>()); break;
			case S8  : std::fill_n((Ctype<S8 >*)entry->host_mem,n,value.get<S8 >()); break;
			case S16 : std::fill_n((Ctype<S16>*)entry->host_mem,n,value.get<S16>()); break;
			case S32 : std::fill_n((Ctype<S32>*)entry->host_mem,n,value.get<S32>()); break;
			case S64 : std::fill_n((Ctype<S64>*)entry->host_mem,n,value.get<S64>()); break;
			default: assert(0);
		}
	}

	return berr;
}

Berr Block::load(IFile *file) {
	TimedRegion region(Runtime::getClock(),READ);
	StreamDir dir = key.node->streamdir();
	NumDim dim = key.node->numdim();

	assert(dir == IN || dir == IO);
	assert(dim != D0 || scalar_page != nullptr); //D0 --> scalar_page
	assert(dim == D0 || entry->dev_mem != nullptr); // !D0 --> entry
	assert(dim == D0 || entry->host_mem != nullptr); // !D0 --> entry

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
	assert(dim != D0 || scalar_page != nullptr); // D0 --> scalar_page
	assert(dim == D0 || entry->dev_mem != nullptr); // !D0 --> entry
	assert(dim == D0 || entry->host_mem != nullptr); // !D0 --> entry

	Ferr ferr = file->writeBlock(*this);

	assert(ferr == 0); // any check?
	Berr berr = ferr; // from ferr to berr

	return berr;
}

void Block::fixValue(VariantType var) {
	assert(not var.isNone());
	fixed = true;
	ready = true;
	value = var;
}

void Block::notify() {
	if (dependencies >= 0)
		dependencies--;
	// Note: blocks with DEPEND_UNKNOWN are never discarded
}

bool Block::discardable() const {
	return dependencies == DEPEND_ZERO; // == 0
}

void Block::setReady() {
	assert(not ready);
	ready = true;
}

bool Block::isReady() {
	return ready;
}

void Block::setDirty() {
	assert(not dirty);
	dirty = true;
	entry->setDirty();
}

void Block::unsetDirty() {
	assert(dirty);
	dirty = false;
	entry->unsetDirty();	
}

bool Block::isDirty() {
	assert(dirty == entry->dirty);
	return dirty;
}

void Block::setUsed() {
	used++;
	if (entry)
		entry->setUsed();
}

void Block::unsetUsed() {
	assert(used > 0);
	used--;
	if (entry)
		entry->unsetUsed();
}

bool Block::isUsed() {
	if (entry)
		assert(used == entry->used);
	return used > 0;
}

void Block::setLoading() {
	assert(not loading);
	loading = true;
}

void Block::unsetLoading() {
	assert(loading);
	loading = false;
}

bool Block::isLoading() {
	return loading;
}

void Block::setWriting() {
	assert(not writing);
	writing = true;
}

void Block::unsetWriting() {
	assert(writing);
	writing = false;
}

bool Block::isWriting() {
	return writing;
}

void Block::waitForLoader() {
	std::unique_lock<std::mutex> lock(mtx,std::adopt_lock);
	cv_load.wait(lock,[&]{ return !isLoading(); }); // exit-condition = !loading
	lock.release();
}

void Block::notifyLoaders() {
	cv_load.notify_all();
}

void Block::waitForWriter() {
	std::unique_lock<std::mutex> lock(mtx,std::adopt_lock);
	cv_write.wait(lock,[&]{ return !isWriting(); }); // exit-condition = !writing
	lock.release();
}

void Block::notifyWriters() {
	cv_write.notify_all();
}
} } // namespace map::detail
