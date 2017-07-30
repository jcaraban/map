/**
 * @file	Cache.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: pinned_list now has 1 cl_mem per worker, it would need 'max_in_block+max_out_block' if events are activated
 * TODO: try 'events' again, make sure events are not re-allocated in the main worker loop (clCreateUserEvent clReleaseEvent)
 * TODO: create a pool of Blocks (like 'pinned_list') so that blocks are reused instead than re-allocated non-stop?
 *
 */

#include "Cache.hpp"
#include "Program.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "block/Block0.hpp"
#include "block/Block1.hpp"
#include "block/BlockN.hpp"
#include "../file/binary.hpp" // @ needed for retainFile
#include <algorithm>
#include "Runtime.hpp"


namespace map { namespace detail {

Cache::Cache(Clock &clock, Config &conf)
	: clock(clock)
	, conf(conf)
{ }

Cache::~Cache() { 
	clear();
}

void Cache::clear() {
	scalar_page = nullptr;
	group_page = nullptr;
	chunk_list.clear();
	entry_list.clear();
	lru_list.clear();
	blk_hash.clear();
	pinned_mem.clear();
	pinned_ptr.clear();

	file_hash.clear();
	file_count.clear();
}

void Cache::allocChunks(cle::Context ctx) {
	TimedRegion region(clock,ALLOC_C);
	cl_int err;

	assert(chunk_list.size() == 0); // can't alloc twice
	assert(scalar_page == nullptr);
	assert(group_page == nullptr);

	// Allocates chunks of entries
	chunk_list.resize(conf.cache_num_chunk);

	for (auto &c : chunk_list) {
		c = clCreateBuffer(*ctx, CL_MEM_READ_WRITE, conf.cache_chunk_size, nullptr, &err);
		cle::clCheckError(err);
	}

	// Allocates the page for block reductions / statistics
	scalar_page = clCreateBuffer(*ctx, CL_MEM_READ_WRITE, conf.cache_block_size, nullptr, &err);
	cle::clCheckError(err);

	// Allocates the page for group reductions / statistics
	group_page = clCreateBuffer(*ctx, CL_MEM_READ_WRITE, conf.cache_group_size, nullptr, &err);
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

	// Releases the page for block reductions / statistics
	err = clReleaseMemObject(scalar_page);
	cle::clCheckError(err);

	// Releases the page for group reductions / statistics
	err = clReleaseMemObject(group_page);
	cle::clCheckError(err);

	// Clears all Cache:: data structures
	clear();
}

void Cache::allocEntries(const Program &prog) {
	TimedRegion region(clock,ALLOC_E);
	cle::Context ctx = Runtime::getOclEnv().C(0);
	cle::Queue que = ctx.Q(0);
	cl_int err;

	assert(not chunk_list.empty()); // Chunks of memory need to be allocated
	assert(scalar_page != nullptr);
	assert(group_page != nullptr);
	assert(entry_list.empty()); // can't alloc twice without freeing before

	// Finds the minimum common size for the cache unit
	unit_mem_size = 0;
	unit_block_size = BlockSize{1,1};//,1,1}; @
	unit_dimension = 0;

	for (auto task : prog.taskList())
	{
		for (auto node : full_join(task->inputList(),task->nodeList())) {
			size_t size = node->metadata().totalBlockSize();
			if (size > unit_mem_size) {
				unit_mem_size = size;
				unit_block_size = node->blocksize();
			}
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

			if (false) { // Cleaning up the device memory. Takes some time, might be good in DEBUG mode ?
				size_t ffff = 0xffffffff;
				err = clEnqueueFillBuffer(*que,subbuf,&ffff,sizeof(ffff),0,unit_mem_size,0,nullptr,nullptr);
				cle::clCheckError(err);
			}

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
	// group is not cleared!
	entry_list.clear();
	lru_list.clear();
	blk_hash.clear();
	pinned_mem.clear();
	pinned_ptr.clear();

	file_hash.clear();
	file_count.clear();
}

//

void Cache::requestBlocks(const KeyList &key_list, BlockList &blk_list) {
	blk_list.clear();
	int idx = 0;
	for (auto &tuple : key_list) {
		Key key = std::get<0>(tuple);
		HoldType hold = std::get<1>(tuple);
		int dep = std::get<2>(tuple);

		if (hold == HOLD_0) // Null block that holds '0' values
		{
			blk_list.push_back( new Block0(key,dep) );
		}
		else if (hold == HOLD_1) // Block holds '1' value
		{
			blk_list.push_back( new Block1(key,dep,scalar_page) );
			blk_list.back()->setFile( retainFile(blk_list.back()->key) ); // @@
		}
		//else if (hold == HOLD_2) // Block holds 'groups per block' values
		//{
		//	assert(0);
		//	blk_list.push_back( new Block2(key,dep,group_page) );
		//	blk_list.back()->setFile( retainFile(blk_list.back()->key) ); // @@
		//}
		else if (hold == HOLD_N) // Normal case, block holds 'N' values
		{
			blk_list.push_back( retainBlock(key,dep) );
		}
		else {
			assert(0);
		}
		// @
		blk_list.back()->order = idx++;
	}
}

void Cache::requestEntries(BlockList &blk_list) {
	for (auto &blk : blk_list)
		if (blk->holdtype() == HOLD_N)
			retainEntry(blk);
}

void Cache::returnEntries(BlockList &blk_list) {
	for (auto &blk : blk_list)
		if (blk->holdtype() == HOLD_N)
			releaseEntry(blk);
}

void Cache::returnBlocks(const KeyList &key_list, BlockList &blk_list) {
	assert(key_list.size() == blk_list.size());
	for (int i=0; i<key_list.size(); i++) {
		auto tuple = key_list[i];
		Key key = std::get<0>(tuple);
		HoldType hold = std::get<1>(tuple);
		Block *blk = blk_list[i];

		if (hold == HOLD_0) // Null block, holds '0' values
		{
			delete blk;
		}
		else if (hold == HOLD_1) // D0 block, holds '1' value
		{
			delete blk;
			//releaseFile(key); // @@
		}
		else if (hold == HOLD_N) // Regular block, holds 'N' values
		{
			releaseBlock(key,blk);
		}
		else {
			assert(0);
		}
	}
	blk_list.clear();
}

// Private methods

Block* Cache::retainBlock(const Key &key, int depend) {
	if (not conf.inmem_cache) { // no-cache mode
		return new BlockN(key,DEPEND_UNKNOWN,unit_mem_size); // always created / deleted
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
		assert(depend > DEPEND_UNKNOWN);

		blk = new BlockN(key,depend,unit_mem_size);
		blk_hash[key] = std::unique_ptr<Block>(blk);

		blk->setFile( retainFile(blk->key) );
	}

	{ // Marks block (and entry if exists) as used
		std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe
		blk->setUsed();
	}

	return blk;
}

std::shared_ptr<IFile> Cache::retainFile(Key key) {
	// Some nodes have their own file (e.g. Read, Write, Identity of IO-node)
	if (key.node->file.get() != nullptr)
		return key.node->file;

	// All other nodes requires a temporal file
	key.coord = Coord(); // file is shared by the whole data range
	std::unique_lock<std::mutex> lock(mtx_file); // thread-safe

	auto it = file_hash.find(key);
	if (it == file_hash.end()) {
		// Creates and hashes 'file'
		auto file = std::shared_ptr<IFile>( IFile::Factory(key.node) );
		it = file_hash.insert({key,file}).first;
		// Calculates and hashes 'count'
		int count = prod(key.node->numblock());
		file_count.insert({key,count});
	}

	return it->second;
}

void Cache::retainEntry(Block *blk) {
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe

	if (blk->needEntry()) {
		assert(not blk->isReady());
		Entry *entry = takeEntry(); // gets one for the block
		entry->used = blk->used; // Entry::used must equals Block::used
		blk->setEntry(entry); // Links block --> entry
	}
}

void Cache::releaseEntry(Block *blk) {
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe

	// Notifying that block has been used
	blk->notify();

	// Discards blocks that will not be used anymore
	if (blk->giveEntry())
	{
		// If this block was evicted to disk, discard the file pages
		if (blk->streamdir() == IO && not blk->isDirty())
		{
			auto *bin_file = dynamic_cast<File<binary>*>( blk->getFile().get() );
			bin_file->discard(blk);
		}
		// Giving the entry of a dirty block avoids stores
		if (blk->isDirty()) {
			blk->unsetDirty();
			clock.incr(DISCARDED);
		}

		// Unlinks and gives back the entry
		blk->getEntry()->block = nullptr;
		dropEntry(blk->getEntry());
		blk->unsetEntry();
	}

	// Removes the 'used' mark
	blk->unsetUsed();
}

void Cache::releaseBlock(const Key &key, Block *blk) {
	if (not conf.inmem_cache) {
		blk->getEntry()->reset();
		delete blk; // Always delete in no-cache mode
		return;
	}
	////////////////////////////
	std::unique_lock<std::mutex> lock(mtx_blk); // thread-safe
	bool erase = false;

	auto it = blk_hash.find(key);
	if (it != blk_hash.end()) { // Found
		Block *blk = it->second.get();
		std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe
		erase = blk->discardable() && not blk->isUsed();
	}

	if (erase) {
		assert(not blk->getEntry());
		blk_hash.erase(key); // deletes those blocks that are not needed anymore
		releaseFile(key);
	}
}

void Cache::releaseFile(Key key) {
	std::unique_lock<std::mutex> lock(mtx_file); // thread-safe
	key.coord = Coord();

	if (file_hash.find(key) == file_hash.end())
		return; // Excludes non-allocated files, i.e. IO-nodes files

	auto it = file_count.find(key);
	assert(it != file_count.end());
	
	it->second--; // Notifies the 'file', i.e. counter--
	if (it->second == 0) // Releases the 'files' when done
		file_hash.erase(key);
}

void* Cache::requestHostMem(ThreadId id) {
	return pinned_ptr[id.proj()];
}

//

Entry* Cache::takeEntry() {
	std::unique_lock<std::mutex> lock(mtx_lru); // thread-safe

	for (auto it=lru_list.begin(); it!=lru_list.end(); it++) {
		Entry *entry = *it;
		if (not entry->isUsed()) // If not used, touches and returns
		{
			entry->setUsed(); // first mark
			assert(entry->used == 1);

			touchEntry(entry);
			if (entry->block) // if the entry was owned by another block,
				entry->block->mtx.lock(); // keep locked until evicted!

			return entry;
		}
	}
	assert(!"Could not find an 'unused' entry in 'lru_list");
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

	entry->unsetUsed(); // last unmark
	assert(entry->used == 0);
}

} } // namespace map::detail
