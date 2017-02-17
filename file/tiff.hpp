/** 
 * @file    tiff.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Format for tiff files
 */

#ifndef MAP_FILE_TIFF_HPP_
#define MAP_FILE_TIFF_HPP_

#include "Format.hpp"
#include "tiffio.h"


namespace map { namespace detail {

/*
 *
 */
class tiff : public IFormat<tiff>
{
	TIFF* handler;

  protected:
	tiff(const tiff& other) = delete;
  	tiff& operator=(const tiff& other) = delete;

  	Ferr getMeta();
	Ferr setMeta();
	Ferr getStats();
	Ferr setStats();
	Ferr setOthers();

  public:
	tiff(MetaData& meta, DataStats& stats);
	~tiff();

	Ferr open(std::string file_path, StreamDir stream_dir);
	Ferr close();

	Ferr read(void* dst, const Coord& beg_coord, const Coord& end_coord);
	Ferr write(const void* src, const Coord& beg_coord, const Coord& end_coord);
    
	//Ferr readBlock(void* dst, const Coord& block_coord);
	//Ferr writeBlock(const void* src, const Coord& block_coord);
	Ferr readBlock(Block &block) const;
	Ferr writeBlock(const Block &block);
};

} } // namespace map::detail

#endif
