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
 * @class Shape
 */
struct DataShape {
	NumDim num_dim; //!< dimension number
	DataSize data_size; //!< Full size or resolution of the raster data
	BlockSize block_size; //!< Size of the blocks partitioning the data
	GroupSize group_size; //!< Size of the groups partitioning the block
	NumBlock num_block; //!< Number of blocks = 'data_size' / 'block_size'
	NumGroup num_group; //!< Number of groups = 'block_size' / 'group_size'

	DataShape();
	DataShape(DataSize ds, BlockSize bs, GroupSize gs);
	bool operator==(const DataShape &other);
	bool encompass(const DataShape &other);
};


/*
 * @class MetaData
 */
struct MetaData {
	StreamDir stream_dir; //!< stream direction
	DataType data_type; //!< data type
	NumDim num_dim; //!< dimension number
	MemOrder mem_order; //!< memory order
	//!< codification ? (e.g. compression like bitpack for bool)
	/*
	DataSize data_size; //!< Full size or resolution of the raster data
	BlockSize block_size; //!< Size of the blocks partitioning the data
	NumBlock num_block; //!< Number of blocks = 'data_size' / 'block_size'
	GroupSize group_size; //!< Size of the groups partitioning the block
	NumGroup num_group; //!< Number of groups = 'block_size' / 'group_size'
	*/
	DataShape data_shape; // @
	// what about cached variables like 'size_t total_data_size' ?

	MetaData();
	MetaData(DataSize ds, DataType dt, MemOrder mo, BlockSize bs, GroupSize gs);

	bool operator==(const MetaData &other);

	StreamDir getStreamDir() const;
	DataType getDataType() const;
	NumDim getNumDim() const;
	MemOrder getMemOrder() const;
	const DataSize& getDataSize() const;
	const BlockSize& getBlockSize() const;
	const NumBlock& getNumBlock() const;
	const GroupSize& getGroupSize() const;
	const NumGroup& getNumGroup() const;
	const DataShape& getDataShape() const;
	/*
	StreamDir& refStreamDir();
	DataType& refDataType();
	NumDim& refNumDim();
	MemOrder& refMemOrder();
	DataSize& refDataSize();
	BlockSize& refBlockSize();
	NumBlock& refNumBlock();
	GroupSize& refGroupSize();
	NumGroup& refNumGroup();
	*/
	void setStreamDir(StreamDir stream_dir);
	void setDataType(DataType data_type);
	void setNumDim(NumDim num_dim);
	void setMemOrder(MemOrder mem_order);
	void setDataSize(DataSize data_size);
	void setBlockSize(BlockSize block_size);
	void setGroupSize(GroupSize group_size);

	size_t totalDataSize() const;
	size_t totalBlockSize() const;
	size_t totalGroupSize() const;
};

} } // namespace map::detail

#endif
