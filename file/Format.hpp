/** 
 * @file    Format.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: using Curiously Recurrently Template Pattern (CRTP)
 * Link: http://stackoverflow.com/a/8375713/1237658
 * Link: http://stackoverflow.com/a/3829887/1237658
 *
 */

#ifndef MAP_FILE_FORMAT_HPP_
#define MAP_FILE_FORMAT_HPP_

#include "MetaData.hpp"
#include "DataStats.hpp"
#include "../runtime/Block.hpp" // @


namespace map { namespace detail {

typedef int Ferr;

/*
 * @class Meta-class IFormat<...>
 * Used as a generic-base-class from which specific <Format> classes inherit static variables
 * Using CRTP
 *
 */
template <typename F>
class IFormat
{
  public:
  	/***********
  	   Support
  	 ***********/

	struct Support {
		struct StreamDir {
			static const bool IN;
			static const bool OUT;
			static const bool IO;
		};
		struct DataType {
			static const bool F32;
			static const bool F64;
			static const bool B8;
			static const bool U8;
			static const bool U16;
			static const bool U32;
			static const bool U64;
			static const bool S8;
			static const bool S16;
			static const bool S32;
			static const bool S64;
		};
		struct NumDim {
			static const bool TIME;
			static const bool D0;
			static const bool D1;
			static const bool D2;
			static const bool D3;
		};
		struct MemOrder {
			static const bool BLK;
			static const bool ROW;
			static const bool COL;
			static const bool SFC;
		};
		/*struct MemAccess {
			static const bool FULL_RANDOM;
			static const bool BLK_RANDOM;
			static const bool SEQUENTIAL;
		};*/
		/*struct Compression {
			static const bool 
		};*/
		struct Parallel {
			static const bool PARAREAD;
			static const bool PARAWRITE;
		};
	};
	
	IFormat(MetaData& meta, DataStats& stats);
	~IFormat();

	//virtual Ferr open(std::string file_path, StreamDir stream_dir) = 0;
	//virtual Ferr close() = 0;

	//virtual Ferr read(void* dst, const Coord& beg_coord, const Coord& end_coord) = 0;
	//virtual Ferr write(const void* src, const Coord& beg_coord, const Coord& end_coord) = 0;
	
	//virtual Ferr readBlock(void* dst, const Coord& block_coord) = 0;
	//virtual Ferr writeBlock(const void* src, const Coord& block_coord) = 0;

  protected:
	MetaData& meta; //!< Aggregation, 'meta' is not owned by the Format
	DataStats& stats; //!< Aggregation, 'stats' is not owned by the Format
};

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_FILE_FORMAT_TPL_
#define MAP_FILE_FORMAT_TPL_

namespace map { namespace detail {

template <typename F>
IFormat<F>::IFormat(MetaData& meta, DataStats& stats)
	: meta(meta)
	, stats(stats)
{ }

template <typename F>
IFormat<F>::~IFormat() { }

} } // namespace map::detail

#endif
