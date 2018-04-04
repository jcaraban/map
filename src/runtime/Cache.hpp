/**
 * @file    Cache.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: There should be 1 cache per physical memory (Dev mem, Host mem, SSD mem, HDD mem)
 * TODO: consider refactorizing the 'block management' functionality out of Cache
 * TODO: consider creating a pool of Blocks, and remove unique_ptr<Block> from the hash
 */

#ifndef MAP_RUNTIME_CACHE_HPP_
#define MAP_RUNTIME_CACHE_HPP_

#include "../ThreadId.hpp"
#include "Entry.hpp"
#include "block/BlockList.hpp"
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>


namespace map { namespace detail {

class Program; // Forward declaration
class Clock; // Forward declaration
class Config; // Forward declaration

typedef std::shared_ptr<IFile> SharedFile;

class Cache
{
  public:
	Cache(Clock &clock, Config &conf);
	~Cache();
	Cache(const Cache&) = delete;
	Cache& operator=(const Cache&) = delete;

	void clear();

	void allocChunks(cle::Context ctx);
	void freeChunks();
	void allocEntries(const Program &prog);
	void freeEntries();

	void requestBlocks(const KeyList &key_list, BlockList &blk_list);
	void requestEntries(BlockList &blk_list);

	void returnBlocks(const KeyList &key_list, BlockList &blk_list);
	void returnEntries(BlockList &blk_list);

	void* requestHostMem(ThreadId id); // @

  private:
  	Block* retainBlock(const Key &k, int depend);
  	Block* retainBlock_(const Key &k, int depend);
	void retainEntry(Block *blk);
	void retainEntry_(Block *blk);
	std::shared_ptr<IFile> retainFile(Key key);

	void releaseEntry(Block *blk);
	void releaseEntry_(Block *blk);
	void releaseBlock(const Key &key, Block *blk);
	void releaseBlock_(const Key &key, Block *blk);
	void releaseFile(Key key);

	Entry* takeEntry();
	void touchEntry(Entry *entry);
	void dropEntry(Entry *entry);

  private:
	Clock &clock; // Aggregate
	Config &conf; // Aggregate

	cl_mem block_page; //!< Page of device memory for block-level reductions / statistics
	cl_mem group_page; //!< Page of device memory for work-group reductions / statistics
	std::vector<cl_mem> chunk_list; //!< Chunks of device memory
	std::vector<Entry> entry_list; //!< Entry memory allocator
	std::list<Entry*> lru_list; //!< Least Recently Used LRU linked list
	std::unordered_map<Key,std::unique_ptr<Block>,key_hash> blk_hash; //!< Hashed cache directory
	
	std::vector<cl_mem> pinned_mem;
	std::vector<void*> pinned_ptr;
	std::vector<int> block_order; // @

	std::mutex mtx_blk, mtx_lru, mtx_file;

	size_t unit_mem_size;
	BlockSize unit_block_size;
	int unit_dimension;

	std::unordered_map<Key,SharedFile,key_hash> file_hash;
	std::unordered_map<Key,int,key_hash> file_count;
};

} } // namespace map::detail

#endif
