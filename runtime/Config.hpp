/**
 * @file	Config.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: change 'group' (i.e work-group) to e.g. 'subblk' (i.e. sub-block) ?
 */

#ifndef MAP_RUNTIME_CONFIG_HPP_
#define MAP_RUNTIME_CONFIG_HPP_

#include <stddef.h>
#include <cassert>


namespace map { namespace detail {

struct Config {
	//// Fixed options, requires recompilation
	const bool debug = false;
	const bool interpreted = false; // Deactivates compilation
	const bool code_fusion = true; // Activates fusion
	const bool inmem_cache = true; // Activates in-memory caching
	const bool compil_cache = true; // Activates compilation cache
	const bool prediction = true; // Activates predicton

	// Max
	const int max_num_machines = 1;
	const int max_num_devices = 4;
	const int max_num_ranks = 16;
	const int max_num_workers = max_num_machines * max_num_devices * max_num_ranks;
	const size_t max_cache_total_size = (size_t)1024*1024*1024 * 16; // GB
	const size_t max_cache_chunk_size = (size_t)1024*1024*1024 * 1; // GB
	const int max_block_size = 1024*1024*sizeof(double); // 8 MB
	const int max_group_size = 16*16*sizeof(double); // 2 KB
	const int max_in_block = 16;
	const int max_out_block = 8;

	// Min
	const int min_num_machines = 1;
	const int min_num_devices = 1;
	const int min_num_ranks = 1;
	const int min_num_workers = min_num_machines * min_num_devices * min_num_ranks;
	const size_t min_cache_total_size = (size_t)1024*1024 * 128; // MB
	const size_t min_cache_chunk_size = (size_t)1024*1024 * 64; // MB
	const int min_block_size = 64*64*sizeof(bool); // 4 KB (page size)
	const int min_group_size = 8*8*sizeof(bool); // 512 B
	const int min_in_block = 0;
	const int min_out_block = 0;

	// Inferred
	const int max_group_x_block = max_block_size / min_group_size;

	// Default
	const int def_num_machines = 1;
	const int def_num_devices = 1;
	const int def_num_ranks = 16;
	const size_t def_cache_total_size = (size_t)1024*1024 * (512*7); // @ 3.5 GB 
	const size_t def_cache_chunk_size = (size_t)1024*1024 * 256; // @ 256 MB
	const size_t def_cache_block_size = max_out_block * max_num_ranks * sizeof(double);
	const size_t def_cache_group_size = max_group_x_block * def_cache_block_size; // @ max_in_block?
	const int def_block_size = 128*128*sizeof(float);
	const int def_group_size = 16*16*sizeof(float);

	// Limits
	const int hard_nodes_limit = 1024;
	const int soft_nodes_limit = 512;
	const int loop_nested_limit = 4;

	//// Mutable options, configurable at runtime
	int num_machines = def_num_machines;
	int num_devices = def_num_devices;
	int num_ranks = def_num_ranks;
	size_t cache_total_size = def_cache_total_size;
	size_t cache_chunk_size = def_cache_chunk_size;
	size_t cache_block_size = def_cache_block_size;
	size_t cache_group_size = def_cache_group_size;
	int block_size = def_block_size;
	int group_size = def_group_size;
	
	// Mutable Inferred
	int num_workers = num_machines * num_devices * num_ranks;
	int cache_num_chunk = cache_total_size / cache_chunk_size; // rounds down
	int cache_num_entry = cache_total_size / block_size;
	int chunk_num_entry = cache_chunk_size / block_size;
	int block_num_entry = cache_block_size / sizeof(double);

  // Methods
	void setNumMachines(int num_machines);
	void setNumDevices(int num_devices);
	void setNumRanks(int num_ranks);
	void setBlockSize(int block_size);
};

inline void Config::setNumMachines(int num_machines) {
	assert(num_machines >= min_num_machines && num_machines <= max_num_machines);
	this->num_machines = num_machines;
	this->num_workers = num_machines * num_devices * num_ranks;
}

inline void Config::setNumDevices(int num_devices) {
	assert(num_devices >= min_num_devices && num_devices <= max_num_devices);
	this->num_devices = num_devices;
	this->num_workers = num_machines * num_devices * num_ranks;
}

inline void Config::setNumRanks(int num_ranks) {
	assert(num_ranks >= min_num_ranks && num_ranks <= max_num_ranks);
	this->num_ranks = num_ranks;
	this->num_workers = num_machines * num_devices * num_ranks;
}

inline void Config::setBlockSize(int block_size) {
	assert(block_size >= min_block_size && block_size <= max_block_size);
	this->block_size = block_size;
	this->cache_num_entry = cache_total_size / block_size; 
	this->chunk_num_entry = cache_chunk_size / block_size;
}

} } // namespace map::detail

#endif
