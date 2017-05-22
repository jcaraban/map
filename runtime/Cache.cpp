/**
 * @file	Cache.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: pinned_list now has 1 cl_mem per worker, it would need 'max_in_block+max_out_block' if events are activated
 * TODO: try 'events' again, make sure events are not re-allocated in the main worker loop (clCreateUserEvent clReleaseEvent)
 *
 * TODO: create a buffer of Blocks (like 'pinned_list') so that HOLD_0/1 blocks are reaused instead than re-allocated non-stop
 * TODO: could the reduction functionality within scalar.cpp be moved to the cache ?
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

	for (auto it : file_hash)
		delete it.second;
	file_hash.clear();
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
	// block is not cleared!
	// group is not cleared!
	entry_list.clear();
	lru_list.clear();
	blk_hash.clear();
	pinned_mem.clear();
	pinned_ptr.clear();

	for (auto it : file_hash)
		delete it.second;
	file_hash.clear();
}

void Cache::requestBlocks(const KeyList &key_list, BlockList &blk_list) {
	blk_list.clear();
	int idx = 0;
	for (auto &tuple : key_list) {
		Key key = std::get<0>(tuple);
		HoldType hold = std::get<1>(tuple);
		int dep = std::get<2>(tuple);

		/**/ if (hold == HOLD_0) // Null block that holds '0' values
		{
			blk_list.push_back( new Block(key) );
		}
		else if (hold == HOLD_1) // Block holds '1' value
		{
			blk_list.push_back( new Block(key,scalar_page,nullptr) );
		}
		//else if (hold == HOLD_2) // Block holds 'groups per block' values
		//{
		//	assert(0);
		//	blk_list.push_back( new Block(key,nullptr,group_page) );
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

void Cache::retainEntries(BlockList &blk_list) {
	for (auto &blk : blk_list)
		if (blk->holdtype() == HOLD_N)
			retainEntry(blk);
}

void Cache::preLoadInputBlocks(BlockList &in_blk_list) {
	for (auto &iblk : in_blk_list)
	{
		if (iblk->holdtype() == HOLD_1)
		{
			loadScalar(iblk);
		}
		//else if (iblk->holdtype() == HOLD_2)
		//{
		//	assert(0);
		//	loadGroups(iblk);
		//}
		else if (iblk->holdtype() == HOLD_N)
		{
			Node *node = iblk->key.node;
			if (node->datastats().active)
			{
				int idx = proj(iblk->key.coord,node->numblock());
				iblk->stats.max  = node->datastats().maxb[idx];
				iblk->stats.mean = node->datastats().meanb[idx];
				iblk->stats.min  = node->datastats().minb[idx];
				iblk->stats.std  = node->datastats().stdb[idx];
				iblk->stats.active = true;
				auto max = VariantType(iblk->stats.max,iblk->datatype());
				auto min = VariantType(iblk->stats.min,iblk->datatype());
				if (max == min) {
					iblk->fixValue(max);
				}
			}
		}
	}
}

void Cache::readInputBlocks(BlockList &in_blk_list) {
	for (auto &iblk : in_blk_list)
	{
		//if (iblk->holdtype() == HOLD_1)
		//{
		//	loadScalar(iblk);
		//}
		//else if (iblk->holdtype() == HOLD_2)
		//{
		//	assert(0);
		//	loadGroups(iblk);
		//}
		if (iblk->holdtype() == HOLD_N)
		{
			readInBlk(iblk);
		}
	}
}

void Cache::initOutputBlocks(BlockList &out_blk_list) {
	for (auto &oblk : out_blk_list)
	{
		if (oblk->holdtype() == HOLD_1)
		{
			initScalar(oblk);
		}
		//else if (oblk->holdtype() == HOLD_2)
		//{
		//	assert(0);
		//	initGroups(oblk);
		//}
	}
}

void Cache::writeOutputBlocks(BlockList &out_blk_list) {
	for (auto &oblk : out_blk_list)
	{
		if (oblk->holdtype() == HOLD_1)
		{
			storeScalar(oblk);
		}
		//else if (oblk->holdtype() == HOLD_2)
		//{
		//	assert(0);
		//	storeGroups(oblk);
		//}
		else if (oblk->holdtype() == HOLD_N)
		{
			writeOutBlk(oblk);
		}
	}
}

void Cache::reduceOutputBlocks(BlockList &out_blk_list) {
	for (auto &oblk : out_blk_list)
		if (oblk->holdtype() == HOLD_1)
			reduceScalar(oblk);
}

void Cache::releaseEntries(BlockList &blk_list) {
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

		/**/ if (hold == HOLD_0) // Null block, holds '0' values
		{
			delete blk;
		}
		else if (hold == HOLD_1) // D0 block, holds '1' value
		{
			delete blk;
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
		return new Block(key,unit_mem_size,DEPEND_UNKNOWN); // always created / deleted
	}
	////////////////////////////
	std::unique_lock<std::mutex> lock(mtx_blk); // thread-safe
	Block *blk = nullptr;

	auto it = blk_hash.find(key);
	if (it != blk_hash.end()) // Found, retrieves the block
	{
//std::cout << key.node->getName() << " : " << key.node->id << " , " << key.iter << " found" << std::endl;
		blk = it->second.get();
	}
	else // Not found, creates a block and hashes it
	{
//std::cout << key.node->getName() << " : " << key.node->id << " , " << key.iter << " not" << std::endl;
		assert(depend > DEPEND_UNKNOWN);
		blk = new Block(key,unit_mem_size,depend);
		blk_hash[key] = std::unique_ptr<Block>(blk);
	}

	return blk;
}

void Cache::retainEntry(Block *blk) {
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe

	// Conditions for a block to require a cache entry
	bool need_entry = blk->entry == nullptr && not blk->fixed;

	if (need_entry) {
		assert(not blk->isReady());
		Entry *entry = getEntry(); // gets one for the block
		entry->block = blk; // Links entry --> block
		blk->entry = entry; // Links block --> entry
	}
	blk->setUsed();
}

void Cache::readInBlk(Block *blk) {
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe

	if (blk->isReady()) // No need to read when the value is 'ready'
	{
		clock.incr(NOT_LOADED);
	}
	else // not 'ready', loads the block from its file
	{
		load(blk);
	}
}

void Cache::writeOutBlk(Block *blk) {
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe

	if (not blk->isReady()) {
		blk->setReady();
		blk->setDirty();
	}

	// Inmediatelly stores 'output blocks', or when cache is deactivated
	if (blk->key.node->isOutput() || !conf.inmem_cache) {
		if (blk->isDirty())
			blk->unsetDirty();
		store(blk);
	} else {
		clock.incr(NOT_STORED);
	}
}

void Cache::releaseEntry(Block *blk) {
	////////////////////////////
	std::unique_lock<std::mutex> lock(blk->mtx); // thread-safe
	auto isTemporal = [](Block *blk){ return !blk->key.node->isInput() && !blk->key.node->isOutput(); };

	// Notifying that block has been used
	blk->notify();

	// Conditions for a block to give its cache entry away
	bool give_entry = blk->discardable() || blk->fixed;

	// Discards blocks that will not be used anymore
	if (give_entry)
	{
		clock.incr(DISCARDED);

		if (blk->entry != nullptr)
		{
			// If this block was evited to disk, discard the file pages
			if (isTemporal(blk) && not blk->isDirty()) 
			{
				auto *bin_file = dynamic_cast<File<binary>*>( getFile(blk->key) );
				bin_file->discard(*blk);
			}

			blk->entry->block = nullptr;
			if (blk->entry->isDirty())
				blk->entry->unsetDirty();
			dropEntry(blk->entry);
		}
	}

	// Removes the 'used' mark
	blk->unsetUsed();

	if (give_entry)
		blk->entry = nullptr;
}

void Cache::releaseBlock(const Key &key, Block *blk) {
	if (not conf.inmem_cache) {
		blk->entry->reset();
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
		assert(blk->entry == nullptr);
		blk_hash.erase(key); // deletes those blocks that are not needed anymore
	}
}

Entry* Cache::getEntry() {
	std::unique_lock<std::mutex> lock(mtx_lru); // thread-safe

	for (auto it=lru_list.begin(); it!=lru_list.end(); it++) {
		Entry *entry = *it;
		if (not entry->isUsed()) // If not used, touches and returns
		{
			entry->setUsed(); // set 'used' while evicting, for security
			touchEntry(entry);
			evict(entry->block);
			entry->unsetUsed(); // unset 'used'
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
}

void Cache::evict(Block *old) {
	if (old == nullptr)
		return; // first use of an Entry
	////////////////////////////
	std::unique_lock<std::mutex> lock(old->mtx); // thread-safe

	if (old->isDirty()) {
		clock.incr(EVICTED);
		clock.decr(NOT_STORED);
		old->unsetDirty();

		mtx_lru.unlock();
		store(old);
		mtx_lru.lock();
	}

	old->unsetReady();
	old->entry = nullptr;
}

IFile* Cache::getFile(Key key) {
	// IONodes have their own file
	IONode *ionode = dynamic_cast<IONode*>(key.node);
	if (ionode != nullptr)
		return ionode->file();

	// All other nodes requires a temporal file
	key.coord = Coord(); // file is shared by the whole data range
	std::unique_lock<std::mutex> lock(mtx_file); // thread-safe

	auto it = file_hash.find(key);
	if (it == file_hash.end())
		it = file_hash.insert({key,IFile::Factory(key.node)}).first;

	return it->second;
}

// Load, Init, Store

void Cache::load(Block *block) {
	assert(block->holdtype() == HOLD_N);
	clock.incr(LOADED);
	//
	IFile *file = getFile(block->key);
	block->host_mem = pinned_ptr[Tid.proj()];
	//
	block->load(file);
	block->send();
	block->setReady();
	//
	block->host_mem = nullptr;
}

void Cache::store(Block *block) {
	assert(block->holdtype() == HOLD_N);
	clock.incr(STORED);
	//
	IFile *file = getFile(block->key);
	block->host_mem = pinned_ptr[Tid.proj()]; 
	//
	block->recv();
	block->store(file);
	//
	block->host_mem = nullptr;
}

// Scalar

void Cache::loadScalar(Block *block) {
	assert(block->holdtype() == HOLD_1);
	assert(block->value.isNone());

	if (block->key.node->isConstant())
	{
		block->fixValue(block->key.node->value);
	}
	else // not constant
	{
		IFile *file = getFile(block->key);
		block->load(file);
	}
}

void Cache::initScalar(Block *block) {
	if (not block->key.node->isReduction()) // from Dx to D0
		return; // no need for initialization
	//
	cle::Queue que = Runtime::getOclEnv().D(Tid.dev()).Q(Tid.rnk());

	block->value = block->key.node->initialValue();

	int offset = sizeof(double) * (conf.max_out_block*Tid.rnk() + block->order);
	size_t dtsz = block->datatype().sizeOf();
	size_t size = dtsz;

	
	{ // Fill just 1 value
	TimedRegion region(Runtime::getClock(),SEND);
	cl_int err = clEnqueueFillBuffer(*que,scalar_page,&block->value.ref(),dtsz,offset,size,0,nullptr,nullptr);
	cle::clCheckError(err);
	}
}


void Cache::storeScalar(Block *block) {
	if (block->key.node->isReduction())
		return;

	assert(block->holdtype() == HOLD_1);
	assert(not block->value.isNone());
	assert(block->fixed);

	IFile *file = getFile(block->key);
	block->store(file);
}

void Cache::reduceScalar(Block *block) {
	if (not block->key.node->isReduction())
		return;

	assert(block->holdtype() == HOLD_1);
	assert(not block->value.isNone());

	cle::Queue que = Runtime::getOclEnv().D(Tid.dev()).Q(Tid.rnk());

	int offset = sizeof(double) * (conf.max_out_block*Tid.rnk() + block->order);
	size_t size = block->datatype().sizeOf();

	{
	TimedRegion region(Runtime::getClock(),RECV);
	cl_int clerr = clEnqueueReadBuffer(*que,scalar_page,CL_TRUE,offset,size,&block->value.ref(),0,nullptr,nullptr);
	cle::clCheckError(clerr);
	}
//std::cout << "reduc " << block->value << " : " << block->key.node->id << " " << block->datatype().toString() << std::endl;

	IFile *file = getFile(block->key);
	block->store(file);

	block->load(file); // @ loading to update the reduced block value
}

// Groups
void Cache::storeGroups(Block *block) {
	assert(0);
}

void Cache::loadGroups(Block *block) {
	assert(0);
}

void Cache::initGroups(Block *block) {
	assert(0);
}

} } // namespace map::detail
