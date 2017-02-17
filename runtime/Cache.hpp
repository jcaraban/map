/**
 * @file    Cache.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: depend=-1 means the block wont be discarded, useful when its future use is unknown (eg Spreading)
 *
 * TODO: There should be 1 cache per physical memory (Dev mem, Host mem, SSD mem, HDD mem)
 */

#ifndef MAP_RUNTIME_CACHE_HPP_
#define MAP_RUNTIME_CACHE_HPP_

#include "Entry.hpp"
#include "Block.hpp"
#include "Config.hpp"
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>


namespace map { namespace detail {

class Program; // Forward declaration
class Clock; // Forward declaration

class Cache
{
  private:
	Program &prog; // Aggregate
	Clock &clock; // Aggregate
	Config &conf; // Aggregate

	cl_mem scalar_page; //!< Page of device memory where scalars reside
	std::vector<cl_mem> chunk_list; //!< Chunks of device memory
	std::vector<Entry> entry_list; //!< Entry memory allocator
	std::list<Entry*> lru_list; //!< Least Recently Used LRU linked list
	std::unordered_map<Key,std::unique_ptr<Block>,key_hash> blk_hash; //!< Hashed cache directory
	
	std::vector<cl_mem> pinned_mem;
	std::vector<void*> pinned_ptr;

	std::mutex mtx, mtx_blk; //, mtx_lru, mtx_load, mtx_write;
	std::condition_variable cv_load, cv_write; //cv_evict

	size_t unit_mem_size;
	BlockSize unit_block_size;
	int unit_dimension;

	std::unordered_map<Node*,IFile*> file_hash; // @
	std::unordered_set<Key,key_hash> first_time; // @ avoids partial-writes to load the 'first time'

  public:
	Cache(Program &prog, Clock &clock, Config &conf);
	~Cache();
	Cache(const Cache&) = delete;
	Cache& operator=(const Cache&) = delete;

	void clear();

	void allocChunks(cle::Context ctx);
	void freeChunks();
	void allocEntries();
	void freeEntries();

	void retainInputBlocks(const InKeyList &in_key, BlockList &in_blk);
	void retainOutputBlocks(const OutKeyList &out_key, BlockList &out_blk);
	void releaseInputBlocks(BlockList &in_blk);
	void releaseOutputBlocks(BlockList &out_blk, const OutKeyList &out_key);

  private:
  	Block* retainEntryForInput(const Key &k);
	Block* retainEntryForOutput(const Key &k, int depend);
	void releaseEntryFromInput(Block *blk);
	void releaseEntryFromOutput(Block *blk);

	Entry* getLRU();
	void makeLRU(Entry *entry);
	void evict(Block *block);
	IFile* getFile(Node *node); // @

	void load(Block *block);
	void loadScalar(Block *block);
	void loadOut(Block *block); // @

	void store(Block *block);
	void storeScalar(Block *block);

	void waitForLoader(Entry *entry);
	void notifyLoaders();

	void waitForWriter(Entry *entry);
	void notifyWriters();
};

} } // namespace map::detail

#endif
