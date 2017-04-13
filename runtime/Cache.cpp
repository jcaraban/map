/**
 * @file	Cache.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: HOLD_0 leads to null-blocks with null-cl_mem. Null-cl_mem tell the kernel when it is in a border case
 * Note: conf.inmem_cache deactivates the in-memory caching (aka always loads and stores)
 * Note: when inmem_cache is deactivated, the cache still allocates memory chunks and behaves like a mem.pool
 *
 * TODO: the reduction functionality within scalar.cpp has to be moved to the cache
 * TODO: pinned_list now has 1 cl_mem per worker, it would need 'max_in_block+max_out_block' if events are activated
 * TODO: try events again in the future, make sure events are de/allocated outside the main worker loop (clCreateUserEvent clReleaseEvent)
 *
 * TODO: could load / store / getFile be moved out of cache ?
 */

#include "Cache.hpp"
#include "Program.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "../file/binary.hpp" // @ needed for getFile
#include <algorithm>
#include "Runtime.hpp"


namespace map { namespace detail {

Cache::Cache(Program &prog, Clock &clock, Config &conf)
	: prog(prog)
	, clock(clock)
	, conf(conf)
{ }

Cache::~Cache() { }

void Cache::clear() {
	scalar_page = nullptr;
	chunk_list.clear();
	entry_list.clear();
	lru_list.clear();
	blk_hash.clear();
	pinned_mem.clear();
	pinned_ptr.clear();

	for (auto it : file_hash)
		delete it.second;
	file_hash.clear();
}

void Cache::allocChunks(cle::Context ctx) {
	TimedRegion region(clock,ALLOC_C);
	cl_int err;

	assert(chunk_list.size() == 0); // can't alloc twice
	assert(scalar_page == nullptr);

	// Allocates chunks of entries
	chunk_list.resize(conf.cache_num_chunk);

	for (auto &c : chunk_list) {
		c = clCreateBuffer(*ctx, CL_MEM_READ_WRITE, conf.cache_chunk, nullptr, &err);
		cle::clCheckError(err);
	}

	// Allocates the chunk of scalars
	scalar_page = clCreateBuffer(*ctx, CL_MEM_READ_WRITE, conf.scalar_size, nullptr, &err);
	cle::clCheckError(err);
}

void Cache::freeChunks() {
	TimedRegion region(clock,FREE_C);
	cl_int err;

	// Returns if there is nothing to free
	if (chunk_list.empty())
		return;

	// Releases chunks of entries
	for (auto &c : chunk_list) {
		err = clReleaseMemObject(c);
		cle::clCheckError(err);
	}

	// Releases the chunk of scalars
	err = clReleaseMemObject(scalar_page);
	cle::clCheckError(err);

	// Clears all Cache:: data structures
	clear();
}

void Cache::allocEntries() {
	TimedRegion region(clock,ALLOC_E);
	cle::Context ctx = Runtime::getOclEnv().C(0);
	cl_int err;

	assert(not chunk_list.empty()); // Chunks of memory need to be allocated
	assert(scalar_page != nullptr);
	assert(entry_list.size() == 0); // can't alloc twice without freeing before

	// Finds the minimum common size for the cache unit
	unit_mem_size = 0;
	unit_block_size = BlockSize{1,1};//,1,1}; @
	unit_dimension = 0;

	for (auto task : prog.taskList()) {
		for (auto i : task->inputList()) {
			size_t sz = i->metadata().getTotalBlockSize();
			if (sz > unit_mem_size) {
				unit_mem_size = sz;
				unit_block_size = i->blocksize();
			}
			//assert(sz == unit_mem_size || i->numdim() == D0);
		}
		for (auto n : task->nodeList()) {
			size_t sz = n->metadata().getTotalBlockSize();
			if (sz > unit_mem_size) {
				unit_mem_size = sz;
				unit_block_size = n->blocksize();
			}
			//assert(sz == unit_mem_size || n->numdim() == D0);
		}
		if (unit_dimension < task->numdim().toInt()) {
			unit_dimension = task->numdim().toInt();
		}
	}

	if (unit_dimension == 0) {
		std::cerr << "Warning: no cache entries allocated!" << std::endl;
		return; // All tasks are D0, no need for in-memory cache
	}

	// Establishes the unit size
	conf.setBlockSize(unit_mem_size);
	entry_list.reserve(conf.cache_num_entry);

	// Allocation of subbuffers & entries
	for (auto &chunk : chunk_list) {
		for (int i=0; i<conf.chunk_num_entry; i++) {
			_cl_buffer_region reg = {i*unit_mem_size,unit_mem_size};
			cl_mem subbuf = clCreateSubBuffer(chunk, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, &reg, &err);
			cle::clCheckError(err);

			// TODO: what would happe if the subbuffer are touched here?

			// Creates Entry, linked to the subbuffer
			entry_list.push_back( Entry(subbuf) );
			lru_list.push_back( &entry_list.back() );
			lru_list.back()->self = std::next(lru_list.rbegin()).base();
		}
	}

	// Allocation of pinned buffers
	pinned_mem.resize(conf.num_ranks);
	pinned_ptr.resize(conf.num_ranks);

	for (int i=0; i<pinned_mem.size(); i++) {
		pinned_mem[i] = clCreateBuffer(*ctx, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, unit_mem_size, nullptr, &err);
		cle::clCheckError(err);
		pinned_ptr[i] = clEnqueueMapBuffer(*ctx.Q(0),pinned_mem[i], CL_TRUE, MAP_READ | MAP_WRITE, 0, unit_mem_size, 0, nullptr, nullptr, &err);
		cle::clCheckError(err);
	}
}

void Cache::freeEntries() {
	TimedRegion region(clock,FREE_E);
	cle::Context ctx = Runtime::getOclEnv().C(0);
	cl_int err;

	// Release of entries. Chunks remain allocated
	for (auto &entry : entry_list) {
		err = clReleaseMemObject(entry.dev_mem);
		cle::clCheckError(err);
	}

	// Release of pinned buffers
	for (int i=0; i<pinned_mem.size(); i++) {
		err = clEnqueueUnmapMemObject(*ctx.Q(0), pinned_mem[i], pinned_ptr[i], 0, nullptr, nullptr);
		cle::clCheckError(err);
		err = clReleaseMemObject(pinned_mem[i]);
		cle::clCheckError(err);
	}

	// chunk is not cleared!
	// scalar is not cleared!
	entry_list.clear();
	lru_list.clear();
	blk_hash.clear();
	pinned_mem.clear();
	pinned_ptr.clear();
	
	for (auto it : file_hash)
		delete it.second;
	file_hash.clear();
}

void Cache::requestInputBlocks(const InKeyList &in_keys, BlockList &in_blk) {
	in_blk.clear();
	for (auto &i : in_keys) {
		Key ikey = std::get<0>(i);
		HoldType hold = std::get<1>(i);
		int dpnd = std::get<2>(i);

		/**/ if (hold == HOLD_0) // Null block that holds 0 values, see note
		{
			in_blk.push_back( new Block(ikey) ); // @
		}
		else if (hold == HOLD_1) // In-D0 case, block holds 1 value
		{
			in_blk.push_back( new Block(ikey,scalar_page) );
		}
		else if (hold == HOLD_N) // Block holds N values, asks Cache system
		{
			in_blk.push_back( retainBlock(ikey,dpnd) );
		}
		else {
			assert(0);
		}
	}
}

void Cache::requestOutputBlocks(const OutKeyList &out_keys, BlockList &out_blk) {
	out_blk.clear();
	for (auto &o : out_keys) {
		Key okey = std::get<0>(o);
		HoldType hold = std::get<1>(o);
		int dpnd = std::get<2>(o);

		/**/ if (hold == HOLD_0) // Null block that holds 0 values, see note
		{
			assert(!"Never supposed to be called");
		}
		else if (hold == HOLD_1) // Out-D0, block holds 1 value
		{
			out_blk.push_back( new Block(okey,scalar_page) );
		}
		else if (hold == HOLD_N) // Block holds N values, asks Cache system
		{
			out_blk.push_back( retainBlock(okey,dpnd) );
		}
		else {
			assert(0);
		}
	}
}

void Cache::retainInputEntries(BlockList &in_blk) {
	for (auto &ib : in_blk)
		if (ib->holdtype() == HOLD_N)
			retainEntry(ib);
}

void Cache::retainOutputEntries(BlockList &out_blk) {
	for (auto &ob : out_blk)
		if (ob->holdtype() == HOLD_N)
			retainEntry(ob);
}

void Cache::readInputBlocks(BlockList &in_blk) {
	for (auto &ib : in_blk)
	{
		if (ib->holdtype() == HOLD_1)
		{
			loadScalar(ib);
		}
		else if (ib->holdtype() == HOLD_N)
		{
			readInBlk(ib);
		}
	}
}

void Cache::writeOutputBlocks(BlockList &out_blk) {
	for (auto &ob : out_blk)
	{
		if (ob->holdtype() == HOLD_1)
		{
			storeScalar(ob);
		}
		else if (ob->holdtype() == HOLD_N)
		{
			writeOutBlk(ob);
		}
	}
}

void Cache::returnInputBlocks(BlockList &in_blk) {
	for (auto &ib : in_blk) {
		/**/ if (ib->holdtype() == HOLD_0) // Null block that holds 0 values, see note
		{
			delete ib;
		}
		else if (ib->holdtype() == HOLD_1) // In-D0, block holds 1 value
		{
			delete ib;
		}
		else if (ib->holdtype() == HOLD_N) // regular case
		{
			releaseInEntry(ib);
		}
		else {
			assert(0);
		}
	}
	in_blk.clear();
}

void Cache::returnOutputBlocks(BlockList &out_blk) {
	for (auto &ob : out_blk) {
		/**/ if (ob->holdtype() == HOLD_0) // Null block case
		{
			assert(!"Never supposed to be called");
		}
		else if (ob->holdtype() == HOLD_1) // Out-D0 case
		{
			delete ob;
		}
		else if (ob->holdtype() == HOLD_N) // regular case
		{
			releaseOutEntry(ob);
		}
		else {
			assert(0);
		}
	}
	out_blk.clear();
}

// Private methods

Block* Cache::retainBlock(const Key &key, int depend) {
	if (not conf.inmem_cache) { // no-cache mode
		return new Block(key,unit_mem_size,DEPEND_UNKNOWN); // always created / deleted
	}	
	////////////////////////////
	std::unique_lock<std::mutex> lock(mtx_blk); // thread-safe
	Block *blk = nullptr;

	auto it = blk_hash.find(key);
	if (it != blk_hash.end()) // Found, retrieves the block
	{
		blk = it->second.get();
	}
	else // Not found, creates a block and hashes it
	{
		blk = new Block(key,unit_mem_size,depend);
		blk_hash[key] = std::unique_ptr<Block>(blk);
	}
	
	return blk;
}

void Cache::retainEntry(Block *blk) {
	if (blk->fixed) {
		return; // No need for an entry when the value is 'fixed'
	}
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe

	if (blk->entry == nullptr) // No entry
	{
		Entry *entry = getEntry(); // gets one for the block
		entry->block = blk; // Links entry --> block
		blk->entry = entry; // Links block --> entry
	}
	blk->setUsed();
}

void Cache::readInBlk(Block *blk) {
	if (blk->isReady()) {
		clock.incr(NOT_LOADED);
		return; // No need to read when the value is 'ready'
	}
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe

	if (blk->isLoading()) // Has a valid entry
	{
		clock.incr(NOT_LOADED);
		blk->waitForLoader(); // wait till other jobs load it from disk
	}
	else // no entry: evicts LRU, takes its entry and load memory
	{
		blk->setLoading();
		blk->waitForWriter();

		load(blk);

		blk->unsetLoading();
		blk->notifyLoaders();
	}
}

void Cache::writeOutBlk(Block *blk) {
	if (false) {
		return;
	}
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe
	
	blk->setReady();
	blk->setDirty();

	// Inmediatelly stores 'output blocks', or when cache is deactivated
	if (blk->key.node->isOutput() || !conf.inmem_cache) {
		blk->unsetDirty();

		blk->setWriting();
		store(blk);
		blk->unsetWriting();
		blk->notifyWriters();
	} else {
		clock.incr(NOT_STORED);
	}
}

void Cache::releaseInEntry(Block *blk) {
	if (not conf.inmem_cache) {
		blk->entry->reset();
		delete blk; // Always delete in no-cache mode
		return;
	}
	////////////////////////////
	bool del_blk = false;
	{
		std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe
		auto isTemporal = [](Block *blk){ return !blk->key.node->isInput() && !blk->key.node->isOutput(); };

		// Notifying that block has been used
		blk->notify();

		// Removes the 'used' mark
		blk->unsetUsed();

		// Discards blocks that will not be used anymore
		if (blk->discardable())
		{	
			clock.incr(DISCARDED);
			del_blk = true; // marks block deletion

			if (blk->entry != nullptr)
			{
				if (!blk->isDirty() && isTemporal(blk)) // was already evicted to disk, then discards file pages
				{
					auto *bin_file = dynamic_cast<File<binary>*>( getFile(blk->key.node) );
					bin_file->discard(*blk);
				}

				blk->entry->reset();
				dropEntry(blk->entry);
				blk->entry = nullptr;
			}
		}
	}
	
	if (del_blk) {
		std::unique_lock<std::mutex> lock(mtx_blk);
		blk_hash.erase(blk->key); // deletes those blocks that are not needed anymore
	}
}

void Cache::releaseOutEntry(Block *blk) {
	if (not conf.inmem_cache) {
		blk->entry->reset();
		delete blk; // Always delete in no-cache mode
		return;
	}
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe
	
	// Removes the 'used' mark
	blk->unsetUsed();

	// what about makeLRU() when the out-block is written and not needed anymore?

	// Releases entry and moves it to the LRU if block.max == min
	if (blk->entry!=nullptr && blk->fixed)
	{
		blk->entry->reset();
		dropEntry(blk->entry);
		blk->entry = nullptr;
	}
	
}

Entry* Cache::getEntry() {
	std::unique_lock<std::mutex> lock(mtx_lru); // thread-safe
	
	// Gets the LRU entry, touches, marks, evicts, returns it
	Entry *entry = *lru_list.begin();
	assert(not entry->isUsed());

	entry->setUsed(); // set 'used' while evicting, for security

	touchEntry(entry);
	evict(entry->block);

	entry->unsetUsed(); // unset 'used'

	return entry;
}

void Cache::touchEntry(Entry *entry) {
	lru_list.erase(entry->self);
	lru_list.push_back(entry);
	entry->self = std::next(lru_list.rbegin()).base();
}

void Cache::dropEntry(Entry* entry) {
	std::unique_lock<std::mutex> lock(mtx_lru); // thread-safe

	lru_list.erase(entry->self);
	lru_list.push_front(entry);
	entry->self = lru_list.begin();
}

void Cache::evict(Block *old) {
	if (old == nullptr)
		return; // first use of an Entry
	clock.incr(EVICTED);
	clock.decr(NOT_STORED);

	old->unsetDirty();
		
	old->mtx.lock();
	mtx_lru.unlock();

	store(old);

	mtx_lru.lock();
	old->mtx.unlock();

	old->entry = nullptr;
}

IFile* Cache::getFile(Node *node) {
	// IONodes have their own file
	IONode *ionode = dynamic_cast<IONode*>(node);
	if (ionode != nullptr)
		return ionode->file();

	// All other nodes requires a temporal file
	std::unique_lock<std::mutex> lock(mtx_file); // thread-safe

	auto it = file_hash.find(node);
	if (it == file_hash.end())
		it = file_hash.insert({node,IFile::Factory(node)}).first;

	return it->second;
}

void Cache::store(Block *block) {
	assert(block->holdtype() == HOLD_N);

	IFile *file = getFile(block->key.node);

	block->entry->host_mem = pinned_ptr[Tid.proj()]; 
	block->recv();
	block->store(file);
	block->entry->host_mem = nullptr;
	clock.incr(STORED);
}

void Cache::storeScalar(Block *block) {
	assert(block->holdtype() == HOLD_1);
	assert(not block->value.isNone());

	IFile *file = getFile(block->key.node);

	block->store(file);
}

void Cache::load(Block *block) {
	assert(block->holdtype() == HOLD_N);

	IFile *file = getFile(block->key.node);

	block->entry->host_mem = pinned_ptr[Tid.proj()];
	block->load(file);
	block->send();
	block->setReady();
	block->entry->host_mem = nullptr;
	clock.incr(LOADED);
}

void Cache::loadScalar(Block *block) {
	assert(block->holdtype() == HOLD_1);
	assert(block->value.isNone());
	
	auto *cnode = dynamic_cast<Constant*>(block->key.node);
	if (cnode != nullptr) { // @
		block->fixValue(cnode->cnst);
		return;
	}

	IFile *file = getFile(block->key.node);

	block->load(file);
}

} } // namespace map::detail
