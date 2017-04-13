/**
 * @file    Cache.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
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

	std::mutex mtx_blk, mtx_lru, mtx_file; //, mtx_load, mtx_write;

	size_t unit_mem_size;
	BlockSize unit_block_size;
	int unit_dimension;

	std::unordered_map<Node*,IFile*> file_hash;

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

	void requestInputBlocks(const InKeyList &in_key, BlockList &in_blk);
	void requestOutputBlocks(const OutKeyList &out_key, BlockList &out_blk);

	void retainInputEntries(BlockList &in_blk);
	void retainOutputEntries(BlockList &out_blk);

	void readInputBlocks(BlockList &in_blk);
	void writeOutputBlocks(BlockList &out_blk);

	void returnInputBlocks(BlockList &in_blk);
	void returnOutputBlocks(BlockList &out_blk);

  private:
  	Block* retainBlock(const Key &k, int depend);
	void retainEntry(Block *blk);

	void readInBlk(Block *blk);
	void writeOutBlk(Block *blk);

	void releaseInEntry(Block *blk);
	void releaseOutEntry(Block *blk);

	Entry* getEntry();
	void touchEntry(Entry *entry);
	void dropEntry(Entry *entry);
	void evict(Block *block);
	IFile* getFile(Node *node);

	void load(Block *block);
	void loadScalar(Block *block);

	void store(Block *block);
	void storeScalar(Block *block);
};

} } // namespace map::detail

#endif
