/**
 * @file	MetaData.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_FILE_METADATA_HPP_
#define MAP_FILE_METADATA_HPP_

#include "../util/util.hpp"
#include <string>


namespace map { namespace detail {

/*
 * @class MetaData
 * Structure containing all metadata related to a File / Node
 * Owned by the File / Node, accessed by many others
 *
 * Note: can be extended with geographical data in a future
 */
struct MetaData {
	StreamDir stream_dir; //!< stream direction 
	DataType data_type; //!< data type
	NumDim num_dim; //!< dimension number
	MemOrder mem_order; //!< memory order
	//!< codification ? (e.g. compression like bitpack for bool)

	DataSize data_size;
	BlockSize block_size;
	NumBlock num_block;
	//size_t total_data_size;
	//size_t total_block_size;

	MetaData();
	MetaData(DataSize data_size, DataType data_type, MemOrder mem_order, BlockSize block_size);

	StreamDir getStreamDir() const;
	DataType getDataType() const;
	NumDim getNumDim() const;
	MemOrder getMemOrder() const;
	const DataSize& getDataSize() const;
	const BlockSize& getBlockSize() const;
	const NumBlock& getNumBlock() const;

	size_t getTotalDataSize() const;
	size_t getTotalBlockSize() const;
};

inline MetaData::MetaData() { }

inline MetaData::MetaData(DataSize data_size, DataType data_type, MemOrder mem_order, BlockSize block_size)
	: stream_dir( IO ) // @ IO by default
	, data_type( data_type )
	, num_dim( DataSize2NumDim(data_size) ) // @
	, mem_order( mem_order )
	, data_size( data_size )
	, block_size( block_size )
	, num_block( (data_size + block_size - 1) / block_size )
{
	//total_data_size = prod(static_cast<Array<size_t>>(data_size)) * sizeofDataType(data_type);
	//total_block_size = prod(static_cast<Array<size_t>>(block_size)) * sizeofDataType(data_type);
}

inline StreamDir MetaData::getStreamDir() const { return stream_dir; }
inline DataType MetaData::getDataType() const { return data_type; }
inline NumDim MetaData::getNumDim() const { return num_dim; }
inline MemOrder MetaData::getMemOrder() const { return mem_order; }
inline const DataSize& MetaData::getDataSize() const { return data_size; }
inline const BlockSize& MetaData::getBlockSize() const { return block_size; }
inline const NumBlock& MetaData::getNumBlock() const { return num_block; }

inline size_t MetaData::getTotalDataSize() const { return prod(static_cast<Array4<size_t>>(data_size)) * data_type.sizeOf(); }
inline size_t MetaData::getTotalBlockSize() const { return prod(static_cast<Array4<size_t>>(block_size)) * data_type.sizeOf(); }

} } // namespace map::detail

#endif
