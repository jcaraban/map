/** 
 * @file    binary.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Format for binary files
 */

#ifndef MAP_FILE_BINARY_HPP_
#define MAP_FILE_BINARY_HPP_

#include "Format.hpp"
#include <string>


namespace map { namespace detail {


/*
 *
 */
class binary : public IFormat<binary>
{
	int fd; //!< file descriptor
	size_t initial_offset, total_data_size, total_block_size; //!< Cached variables

  protected:
	binary(const binary& other) = delete;
  	binary& operator=(const binary& other) = delete;

  	Ferr getMeta();
	Ferr setMeta();
	Ferr getStats();
	Ferr setStats();

  public:
	binary(MetaData& meta, DataStats& stats);
	~binary();

	Ferr open(std::string file_path, StreamDir stream_dir);
	Ferr create_temp_file();
	Ferr close();

	Ferr read(void* dst, const Coord& beg_coord, const Coord& end_coord);
	Ferr write(const void* src, const Coord& beg_coord, const Coord& end_coord);
    
	//Ferr readBlock(void* dst, const Coord& block_coord);
	//Ferr writeBlock(const void* src, const Coord& block_coord);
	Ferr readBlock(Block &block) const;
	Ferr writeBlock(const Block &block);

	Ferr discard(const Block &block);
};

} } // namespace map::detail

#endif
