/**
 * @file	Block.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Block.hpp"
#include "Runtime.hpp"
#include <algorithm>


namespace map { namespace detail {

/*******
   Key
 *******/

Key::Key()
	: node(nullptr)
	, coord{-1,-1} // @
{ }

Key::Key(Node *node, Coord coord)
	: node(node)
	, coord(coord)
{ }

bool Key::operator==(const Key& k) const {
	return (node==k.node && all(coord==k.coord));
}

std::size_t key_hash::operator()(const Key &k) const {
	/*std::size_t h = std::hash<Task*>()(k.node);
	for (int i=0; i<k.coord.size(); i++)
		h ^= std::hash<int>()(k.coord[i]);*/
	std::size_t h = (size_t)k.node & 0x00000000ffffffff;
	for (int i=0; i<k.coord.size(); i++)
		h |= (size_t)k.coord[i] << (32+i*16);
	return h;
}

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
	, stats()
	, total_size(-1)
	, dependencies(DEPEND_UNKNOWN)
	, hold_type()
{ }

Block::Block(Key key)
	: key(key)
	, entry(nullptr)
	, scalar_page(nullptr)
	, value()
	, fixed(false)
	, stats()
	, total_size(-1)
	, dependencies(DEPEND_UNKNOWN)
	, hold_type(HOLD_0)
{ }

Block::Block(Key key, cl_mem scalar_page)
	: key(key)
	, entry(nullptr)
	, scalar_page(scalar_page)
	, value()
	, fixed(false)
	, stats()
	, total_size(-1)
	, dependencies(DEPEND_UNKNOWN)
	, hold_type(HOLD_1)
{
	assert(numdim() == D0);
}

Block::Block(Key key, int max_size, int depend)
	: key(key)
	, entry(nullptr)
	, scalar_page(nullptr)
	, value()
	, fixed(false)
	, stats()
	, total_size(-1)
	, dependencies(depend)
	, hold_type(HOLD_N)
{
	assert(numdim() != D0);
	
	this->total_size = prod(key.node->blocksize()) * datatype().sizeOf();
	assert(total_size <= max_size);
	assert(max_size % total_size == 0);
}

Block::~Block() { }

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

void Block::notify() {
	if (dependencies >= 0)
		dependencies--;
	// Note: blocks with DEPEND_UNKNOWN are never discarded
}

bool Block::discardable() const {
	return dependencies == DEPEND_ZERO; // == 0
}

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

} } // namespace map::detail
