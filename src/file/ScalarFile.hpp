/** 
 * @file    scalar.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Fake file format, used when the data is invariably composed by a scalar value
 *
 * TODO: scalar could become some sort of Acummulator class with readBlock / writeBlock / setRedTye / value, no more
 * TODO: maybe change the mutex for atomic operations
 */

#ifndef MAP_FILE_SCALAR_HPP_
#define MAP_FILE_SCALAR_HPP_

#include "Format.hpp"
#include "../util/util.hpp"
#include <mutex>


namespace map { namespace detail {

struct Block; // forward declaration

/*
 *
 */
class scalar : public IFormat<scalar>
{
  public: // @
	ReductionType type;
	mutable VariantType val; // @
	std::mutex mtx;

  protected:
	scalar(const scalar& other) = delete;
  	scalar& operator=(const scalar& other) = delete;

  public:
	scalar(MetaData& meta, DataStats& stats);
	~scalar();

	Ferr open(std::string file_path, StreamDir stream_dir) { return 0; } // @
	Ferr close() { return 0; } // @

	Ferr read(void* dst, const Coord& beg_coord, const Coord& end_coord) { return -1; } // @
	Ferr write(const void* src, const Coord& beg_coord, const Coord& end_coord) { return -1; } // @
    
	Ferr readBlock(Block *block) const;
	Ferr writeBlock(const Block *block);

	Ferr setReductionType(const ReductionType &type);
	VariantType value() const;
};

} } // namespace map::detail

#endif
