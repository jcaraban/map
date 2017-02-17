/**
 * @file	Cache.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: HOLD_0 leads to null-blocks with null-cl_mem. Null-cl_mem tell the kernel when it is in a border case
 * Note: conf.inmem_cache deactivates the in-memory caching (aka always loads and stores)
 * Note: when inmem_cache is deactivated, the cache still allocates memory chunks and behaves like a pool
 *
 * TODO: the reduction functionality within scalar.cpp has to be moved to the cache
 * TODO: pinned_list now has 1 cl_mem per worker, it would need 'max_in_block+max_out_block' if events are activated
 * TODO: try events again in the future, make sure events are de/allocated outside the main worker loop (clCreateUserEvent clReleaseEvent)
 * TODO: waitForXXX functions should be relative to blocks, not to entries
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

	first_time.clear();
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
	first_time.clear();
}

void Cache::retainInputBlocks(const InKeyList &in_keys, BlockList &in_blk) {
	in_blk.clear();
	for (auto &i : in_keys) {
		Key ikey = std::get<0>(i);
		HoldType hold = std::get<1>(i);

		/**/ if (hold == HOLD_0) // Null block that holds 0 values, see note
		{
			in_blk.push_back( new Block(ikey) ); // @
		}
		else if (hold == HOLD_1) // In-D0 case, block holds 1 value
		{
			in_blk.push_back( new Block(ikey,scalar_page) );
			loadScalar(in_blk.back());
		}
		else if (hold == HOLD_N) // Block holds N values, asks Cache system
		{
			in_blk.push_back( retainEntryForInput(ikey) );
		}
		else {
			assert(0);
		}
	}
}

void Cache::retainOutputBlocks(const OutKeyList &out_keys, BlockList &out_blk) {
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
			out_blk.push_back( retainEntryForOutput(okey,dpnd) );
		}
		else {
			assert(0);
		}
	}
}

void Cache::releaseInputBlocks(BlockList &in_blk) {
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
			releaseEntryFromInput(ib);
		}
		else {
			assert(0);
		}
	}
	in_blk.clear();
}

void Cache::releaseOutputBlocks(BlockList &out_blk, const OutKeyList &out_keys) {
	for (auto &ob : out_blk) {
		/**/ if (ob->holdtype() == HOLD_0) // Null block case
		{
			assert(!"Never supposed to be called");
		}
		else if (ob->holdtype() == HOLD_1) // Out-D0 case
		{
			storeScalar(ob);
			delete ob;
		}
		else if (ob->holdtype() == HOLD_N) // regular case
		{
			releaseEntryFromOutput(ob);
		}
		else {
			assert(0);
		}
	}
	out_blk.clear();
}

Block* Cache::retainEntryForInput(const Key &key) {
	Block *blk = nullptr;
	mtx_blk.lock(); // thread-safe

	if (not conf.inmem_cache) // no-cache mode
	{
		blk = new Block(key,unit_mem_size,DEPEND_UNKNOWN); // always created / deleted
	}
	else // Normal mode, cache activated
	{
		auto it = blk_hash.find(key);
		if (it != blk_hash.end()) // Found, retrieves block
		{
			blk = it->second.get();
		}
		else // Not found, needs new Block
		{
			blk = new Block(key,unit_mem_size,DEPEND_UNKNOWN);
			blk_hash[key] = std::unique_ptr<Block>(blk);
		}
	}
	
	mtx_blk.unlock();
	mtx.lock(); // thread-safe

	if (blk->entry != nullptr) // Has a valid entry
	{
		clock.incr(NOT_LOADED);
		blk->entry->setUsed();
		waitForLoader(blk->entry); // wait till other jobs load it from disk
	}
	else if (blk->fixed) // The block doesn't need an entry when the value is fixed
	{
		clock.incr(NOT_LOADED);
	}
	else // no entry: evicts LRU, takes its entry and load memory
	{
		Entry *entry = getLRU();
		Block *old_blk = entry->block;

		entry->block = blk;
		blk->entry = entry;

		entry->setUsed();
		entry->setLoading();

		evict(old_blk);
		load(blk);

		entry->unsetLoading();
		notifyLoaders();
	}

	mtx.unlock();
	return blk;
}

Block* Cache::retainEntryForOutput(const Key &key, int depend) {
	Block *blk = nullptr;
	mtx_blk.lock(); // thread-safe

	if (not conf.inmem_cache) // no-cache mode
	{
		blk = new Block(key,unit_mem_size,DEPEND_UNKNOWN); // always created / deleted
	}
	else // Normal mode, cache activated
	{
		auto it = blk_hash.find(key);
		if (it != blk_hash.end()) // Found, retrieves block
		{
			blk = it->second.get();
		}
		else // Not found, needs new Block
		{
			blk = new Block(key,unit_mem_size,depend);
			blk_hash[key] = std::unique_ptr<Block>(blk);
		}
	}
	
	mtx_blk.unlock();
	mtx.lock(); // thread-safe

	if (blk->entry != nullptr) // Has a valid entry
	{
		//clock.incr(NOT_LOADED);
		blk->entry->setUsed();
		waitForLoader(blk->entry);
		waitForWriter(blk->entry); // wait till other jobs finish writing
		blk->entry->setWriting();
	}
	else if (blk->fixed) // The block doesn't need an entry when the value is fixed
	{
		assert(!"Not supposed to reach here");
	}
	else // not found: evicts LRU, takes its entry and load memory
	{
		Entry *entry = getLRU();
		Block *old_blk = entry->block;

		entry->block = blk;
		blk->entry = entry;
		
		entry->setUsed();
		entry->setLoading();

		evict(old_blk);
		loadOut(blk);

		entry->unsetLoading();
		notifyLoaders();
		entry->setWriting(); // TODO: check for race conditions
	}

	mtx.unlock();
	return blk;
}

void Cache::releaseEntryFromInput(Block *blk) {
	mtx.lock(); // thread-safe
	Entry *entry = blk->entry; // Saves pointer in case 'blk' is discarded

	// Notifying that block has been used
	blk->notify();

	// Discarding. Avoids evicting blocks that will not be used anymore
	if (blk->discardable()) {
		clock.incr(DISCARDED);
		// NOTE: binary::discard is not optimal on linux kernel < 4.6
		auto *bin_file = dynamic_cast<File<binary>*>( getFile(blk->key.node) );
		bin_file->discard(*blk);
		
		if (blk->entry != nullptr) {
			makeLRU(blk->entry);
			blk->entry->unsetDirty();
			blk->entry->block = nullptr;
			blk->entry = nullptr;
		}
		
		mtx_blk.lock();
		blk_hash.erase(blk->key); // deletes blocks that won't be needed anymore
		mtx_blk.unlock();
	}
	
	if (entry != nullptr)
		entry->unsetUsed();

	if (not conf.inmem_cache) {
		delete blk; // Always delete in no-cache mode
		entry->block = nullptr;
	}

	mtx.unlock();
}

void Cache::releaseEntryFromOutput(Block *blk) {
	mtx.lock(); // thread-safe
	Entry *entry = blk->entry; // Saves pointer in case 'blk' discards 'entry'
	
	blk->entry->setDirty();

	// Inmediatelly stores 'output blocks', or when cache is deactivated
	if (blk->key.node->isOutput() || !conf.inmem_cache) {
		store(blk);
		blk->entry->unsetDirty();
	} else {
		clock.incr(NOT_STORED);
	}

	// what about makeLRU() when the out-block is written and not needed anymore?

	// Releases entry and moves it to the LRU if block.max == min
	if (blk->fixed) {
		makeLRU(blk->entry);
		blk->entry->unsetDirty();
		blk->entry->block = nullptr;
		blk->entry = nullptr;
	}

	entry->unsetWriting();
	notifyWriters();
	
	entry->unsetUsed();

	if (not conf.inmem_cache) {
		delete blk;
		entry->block = nullptr;
	}

	mtx.unlock();
}

Entry* Cache::getLRU() {
	//std::unique_lock<std::mutex> lock(mtx_lru); // thread-safe

	for (auto it=lru_list.begin(); it!=lru_list.end(); it++) {
		Entry *entry = *it;
		if (not entry->isUsed()) { // If not used, touches and returns
			lru_list.erase(it);
			lru_list.push_back(entry);
			entry->self = std::next(lru_list.rbegin()).base();
			return lru_list.back();
		}
	}
	assert(!"Reached end of 'list' without finding free LRU");
}

void Cache::makeLRU(Entry* entry) {
	//std::unique_lock<std::mutex> lock(mtx_lru); // thread-safe

	lru_list.erase(entry->self);
	lru_list.push_front(entry);
	entry->self = lru_list.begin();
}

void Cache::evict(Block *old) {
	if (old == nullptr)
		return;
	if (old->entry == nullptr)
		return;
	if (old->entry->isDirty()) {
		old->entry->unsetDirty();
		Block copy = *old;
		old->entry = nullptr;
		store(&copy);
		clock.incr(EVICTED);
		clock.decr(NOT_STORED);
	} else {
		old->entry = nullptr;
	}
}

IFile* Cache::getFile(Node *node) { // @
	// IONodes have their own file
	IONode *ionode = dynamic_cast<IONode*>(node);
	if (ionode != nullptr) return ionode->file();
	// All other nodes requires a temporal file
	auto it = file_hash.find(node);
	if (it == file_hash.end())
		it = file_hash.insert({node,IFile::Factory(node)}).first;
	return it->second;
}

void Cache::store(Block *block) {
	assert(block->holdtype() == HOLD_N);

	IFile *file = getFile(block->key.node);
	mtx.unlock(); // No thread-safe, to achieve concurrent IO

	block->entry->host_mem = pinned_ptr[Tid.proj()]; 
	block->recv();
	block->store(file);
	block->entry->host_mem = nullptr;
	clock.incr(STORED);

	mtx.lock(); // thread-safe again
}

void Cache::storeScalar(Block *block) {
	assert(block->holdtype() == HOLD_1);

	mtx.lock(); // thread-safe
	IFile *file = getFile(block->key.node);
	mtx.unlock();

	block->store(file);
}

void Cache::load(Block *block) {
	assert(block->holdtype() == HOLD_N);

	IFile *file = getFile(block->key.node);
	mtx.unlock(); // No thread-safe, to achieve concurrent IO

	block->entry->host_mem = pinned_ptr[Tid.proj()];
	block->load(file);
	block->send();
	block->entry->host_mem = nullptr;
	clock.incr(LOADED);

	mtx.lock(); // thread-safe again
}

void Cache::loadScalar(Block *block) {
	assert(block->holdtype() == HOLD_1);
	// @
	auto *cnode = dynamic_cast<Constant*>(block->key.node);
	if (cnode != nullptr) {
		block->value = cnode->cnst;
		block->fixed = true;
		return;
	}

	mtx.lock(); // thread-safe
	IFile *file = getFile(block->key.node);
	mtx.unlock();

	block->load(file);
}

void Cache::loadOut(Block *block) { // @@ load_out is not yet needed
	return; // @@
	
	if (first_time.find(block->key) == first_time.end()) {
		first_time.insert(block->key);
		return; // @ does not load the first request of an Output-Block
	}
	load(block);
}

void Cache::waitForLoader(Entry *entry) {
	std::unique_lock<std::mutex> lock(mtx,std::adopt_lock);
	cv_load.wait(lock,[&]{ return !entry->isLoading(); }); // exit-condition = !loading
	lock.release();
}

void Cache::notifyLoaders() {
	cv_load.notify_all();
}

void Cache::waitForWriter(Entry *entry) {
	std::unique_lock<std::mutex> lock(mtx,std::adopt_lock);
	cv_write.wait(lock,[&]{ return !entry->isWriting(); }); // exit-condition = !writing
	lock.release();
}

void Cache::notifyWriters() {
	cv_write.notify_all();
}

} } // namespace map::detail
