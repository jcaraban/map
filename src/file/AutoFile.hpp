/** 
 * @file    fauto.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Uses GDAL to automatically deal with any file format
 */

#ifndef MAP_FILE_FAUTO_HPP_
#define MAP_FILE_FAUTO_HPP_

#include "../file/Format.hpp"
#include "gdal/gdal_priv.h"
#include <string>


namespace map { namespace detail {

class fauto : public IFormat<fauto> {
	GDALDataset *dataset;
	GDALRasterBand *band;
	GDALDataType type;

  public:
	fauto(MetaData& meta);
	~fauto();

	fauto(const fauto& other) = delete;
	fauto& operator=(const fauto& other) = delete;

	Ferr open(std::string file_path, StreamDir stream_dir);
	Ferr close();

	Ferr read(void* dst, const Coord& beg_coord, const Coord& end_coord);
	Ferr write(const void* src, const Coord& beg_coord, const Coord& end_coord);
	
	Ferr readBlock(void* dst, const Coord& block_coord);
	Ferr writeBlock(const void* src, const Coord& block_coord);
};

} } // namespace map::detail

#endif
