/**
 * @file	BlockN.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "BlockN.hpp"
#include "../../intermediate/Node.hpp"
#include "../../Runtime.hpp"


namespace map { namespace detail {

BlockN::BlockN()
	: Block()
	, entry(nullptr)
	, host_mem(nullptr)
	, file(nullptr)
	, value()
	, fixed(false)
	, forward(false)
	, stats()
	, total_size(-1)
{ }

BlockN::BlockN(Key key, int dep, int max_size)
	: Block(key,dep)
	, entry(nullptr)
	, host_mem(nullptr)
	, file(nullptr)
	, value()
	, fixed(false)
	, forward(false)
	, stats()
	, total_size(-1)
{
	this->total_size = prod(node()->blocksize()) * datatype().sizeOf();
	assert(total_size <= max_size);
	assert(max_size % total_size == 0);
}

BlockN::~BlockN() { }

int BlockN::size() const {
	return total_size;
}

HoldType BlockN::holdtype() const {
	return HOLD_N;
}

Berr BlockN::preload() {
	std::unique_lock<std::mutex> lock(mtx); // thread-safe
	if (isReady()) 
		return 0;
	// If statistics exist, preload is possible
	if (node()->datastats().active)
		setStats( node()->datastats().get(coord()) );
	return 0;
}

Berr BlockN::evict() {
	std::unique_lock<std::mutex> lock(mtx); // thread-safe

	if (entry == nullptr) { // no entry to evict
		assert(isFixed() || isForward());
		return 0;
	}

	Block *old = entry->block;
	if (old == this) // already owner, nothing to do
		return 0; 

	if (old != nullptr) {
		Runtime::getClock().incr(EVICTED);

		if (old->isDirty()) {
			Runtime::getClock().decr(NOT_STORED);
			Runtime::getClock().incr(STORED);

			BlockN *oldN = dynamic_cast<BlockN*>(old);
			oldN->setHostMem( Runtime::getCache().requestHostMem(Tid) );
			oldN->recv();
			oldN->write();
			oldN->unsetDirty();
			old->unsetHostMem();
		}

		old->unsetReady();
		old->unsetEntry();

		old->unsetHostMem();
		old->mtx.unlock(); // finally unlocked!
	}

	entry->block = this; // Links entry --> block
	return 0;
}

Berr BlockN::load() {
	std::unique_lock<std::mutex> lock(mtx); // thread-safe

	if (isReady()) { // No need to read when the value is 'ready'
		Runtime::getClock().incr(NOT_LOADED);
		return 0;
	}
	// Otherwise reads the block from its file
	setHostMem( Runtime::getCache().requestHostMem(Tid) );
	read();
	send();
	setReady();
	unsetHostMem();

	Runtime::getClock().incr(LOADED);
	return 0;
}

Berr BlockN::store() {
	std::unique_lock<std::mutex> lock(mtx); // thread-safe

	if (not isReady())
		setReady();

	if (streamdir() == IN) { // Nothing to store for IN streams
		return 0; // e.g. Identity of a Read node
	}

	setDirty();

	// No need to store when inmem_cache=1 and not an Output node
	if (not node()->isOutput() && Runtime::getConfig().inmem_cache) {
		Runtime::getClock().incr(NOT_STORED);
		return 0;
	}
	// Otherwise write to file and clean the state
	setHostMem( Runtime::getCache().requestHostMem(Tid) );
	recv();
	write();
	unsetDirty();
	unsetHostMem();

	Runtime::getClock().incr(STORED);
	return 0;
}

Berr BlockN::send() {
	TimedRegion region(Runtime::getClock(),SEND);

	assert(not fixed);
	if (not fixed) {
		cle::Queue que = Runtime::getOclEnv().D(Tid.dev()).Q(Tid.rnk());
		cl_int clerr = clEnqueueWriteBuffer(*que, entry->dev_mem, CL_TRUE, 0, size(), host_mem, 0, nullptr, nullptr);
		cle::clCheckError(clerr);
	}

	return 0;
}

Berr BlockN::recv() {
	TimedRegion region(Runtime::getClock(),RECV);

	if (not fixed) {
		cle::Queue que = Runtime::getOclEnv().D(Tid.dev()).Q(Tid.rnk());
		cl_int clerr = clEnqueueReadBuffer(*que, entry->dev_mem, CL_TRUE, 0, size(), host_mem, 0, nullptr, nullptr);
		cle::clCheckError(clerr);
	} else {
		const size_t num = prod(key.node->blocksize());
		value.fill(host_mem,num); // @ move to File::write
	}

	return 0;
}

Berr BlockN::read() {
	TimedRegion region(Runtime::getClock(),READ);
//std::cout << "reading " << node()->id << " " << coord() << std::endl;
	Ferr ferr = file->readBlock(this);
	return 0;
}

Berr BlockN::write() {
	TimedRegion region(Runtime::getClock(),WRITE);
//std::cout << "writing " << node()->id << " " << coord() << std::endl;
	Ferr ferr = file->writeBlock(this);
	return 0;
}

bool BlockN::needEntry() const {
	// Conditions for a block to require a cache entry
	return !entry && !isFixed() && !isForward();
}

bool BlockN::giveEntry() const {
	// Conditions for a block to give its cache entry away
	return entry && (discardable() || isFixed());
}

Entry* BlockN::getEntry() const {
	return entry;
}

void BlockN::setEntry(Entry *entry) {
	this->entry = entry;
}

void BlockN::unsetEntry() {
	this->entry = nullptr;
}

std::shared_ptr<IFile> BlockN::getFile() const {
	return file;
}

void BlockN::setFile(std::shared_ptr<IFile> file) {
	this->file = file;
}

void* BlockN::getHostMem() const {
	return host_mem;
}

void BlockN::setHostMem(void *host_mem) {
	this->host_mem = host_mem;
}

void BlockN::unsetHostMem() {
	this->host_mem = nullptr;
}

void* BlockN::getDevMem() {
	return (entry) ? entry->dev_mem : nullptr;
}

CellStats BlockN::getStats() const {
	return stats;
}

void BlockN::setStats(CellStats sta) {
	assert(sta.active);
	assert(sta.data_type == datatype());

	stats.data_type = key.node->datatype();
	stats.num_group = key.node->numgroup();
	stats.active = true;
	stats.min = sta.min;
	stats.max = sta.max;
	stats.mean = sta.mean;
	stats.std = sta.std;

	// If the value range is fixed, fix the block
	if (sta.max == sta.min) {
		fixValue(sta.max);
	}
}

VariantType BlockN::getValue() const {
	return value;
}
void BlockN::setValue(VariantType val) {
	assert(val.datatype() == datatype());
	this->value = val;
}

bool BlockN::isFixed() const {
	return fixed;
}

void BlockN::fixValue(VariantType val) {
	assert(not val.isNone());
	assert(val.datatype() == datatype());
	fixed = true;
	if (not isReady())
		setReady();
	setValue(val);
}

void BlockN::setForward() {
	assert(not forward);
	forward = true;
}

void BlockN::unsetForward() {
	assert(forward);
	forward = false;
}

bool BlockN::isForward() const {
	return forward;
}

void BlockN::forwardEntry(Block *out) {
	// in_blk entry must be valid
	assert(this->entry);
	// forward only to the last dependency
	assert(this->depend == 1);
	// out_blk entry must be null
	assert(not out->getEntry());

	// Swaps entries
	out->setEntry(getEntry());
	unsetEntry();

	out->getEntry()->block = out;
	out->getEntry()->dirty = false; // @ mmm
}

} } // namespace map::detail
