/**
 * @file	MetaData.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "MetaData.hpp"
#include <string>


namespace map { namespace detail {

// Shape

DataShape::DataShape() { }

DataShape::DataShape(DataSize data_size, BlockSize block_size, GroupSize group_size)
	: num_dim( DataSize2NumDim(data_size) ) // @
	, data_size( data_size )
	, block_size( block_size )
	, group_size( group_size )
{
	num_block = (data_size + block_size - 1) / block_size;
	num_group = (block_size + group_size - 1) / group_size;
}

bool DataShape::operator==(const DataShape &other) {
	bool b = true;
	b = b && all( this->data_size == other.data_size );
	b = b && all( this->block_size == other.block_size );
	b = b && all( this->group_size == other.group_size );
	return b;
}

bool DataShape::encompass(const DataShape &other) {
	if (other.num_dim == NONE_NUMDIM)
		return true;
	if (num_dim == NONE_NUMDIM)
		return false;
	if (other.num_dim.toInt() > num_dim.toInt())
		return false;
	if (num_dim.toInt() > other.num_dim.toInt())
		return true;
	if (num_dim != D0) {
		if (any(other.data_size > data_size))
			return false;
		if (any(other.block_size > block_size))
			return false;
		if (any(other.group_size > group_size))
			return false;
	}
	return true;
}

// Meta

MetaData::MetaData() { }

MetaData::MetaData(DataSize data_size, DataType data_type, MemOrder mem_order, BlockSize block_size, GroupSize group_size)
	: stream_dir( IO ) // @ IO by default
	, data_type( data_type )
	, num_dim( DataSize2NumDim(data_size) ) // @
	, mem_order( mem_order )
	/*
	, data_size( data_size )
	, block_size( block_size )
	, num_block( (data_size + block_size - 1) / block_size )
	, group_size( group_size )
	, num_group( (block_size + group_size - 1) / group_size )
	*/
	, data_shape(data_size,block_size,group_size)
{
	//total_data_size = prod(static_cast<Array<size_t>>(data_size)) * sizeofDataType(data_type);
	//total_block_size = prod(static_cast<Array<size_t>>(block_size)) * sizeofDataType(data_type);
}

bool MetaData::operator==(const MetaData &other) {
	bool b = true;
	b = b && this->stream_dir == other.stream_dir;
	b = b && this->data_type == other.data_type;
	b = b && this->num_dim == other.num_dim;
	b = b && this->mem_order == other.mem_order;
	/*
	b = b && all( this->data_size == other.data_size );
	b = b && all( this->block_size == other.block_size );
	b = b && all( this->num_block == other.num_block );
	b = b && all( this->group_size == other.group_size );
	b = b && all( this->num_group == other.num_group );
	*/
	b = b && data_shape == other.data_shape;
	return b;
}

StreamDir MetaData::getStreamDir() const {
	return stream_dir;
}

DataType MetaData::getDataType() const {
	return data_type;
}

NumDim MetaData::getNumDim() const {
	return num_dim;
}

MemOrder MetaData::getMemOrder() const {
	return mem_order;
}

const DataSize& MetaData::getDataSize() const {
	return data_shape.data_size;
}

const BlockSize& MetaData::getBlockSize() const {
	return data_shape.block_size;
}

const NumBlock& MetaData::getNumBlock() const {
	return data_shape.num_block;
}

const GroupSize& MetaData::getGroupSize() const {
	return data_shape.group_size;
}

const NumBlock& MetaData::getNumGroup() const {
	return data_shape.num_group;
}

const DataShape& MetaData::getDataShape() const {
	return data_shape;
}
/*
StreamDir& MetaData::refStreamDir() {
	return stream_dir;
}

DataType& MetaData::refDataType() {
	return data_type;
}

NumDim& MetaData::refNumDim() {
	return num_dim;
}

MemOrder& MetaData::refMemOrder() {
	return mem_order;
}

DataSize& MetaData::refDataSize() {
	return data_shape.data_size;
}

BlockSize& MetaData::refBlockSize() {
	return data_shape.block_size;
}

NumBlock& MetaData::refNumBlock() {
	return data_shape.num_block;
}

GroupSize& MetaData::refGroupSize() {
	return data_shape.group_size;
}

NumGroup& MetaData::refNumGroup() {
	return data_shape.num_group;
}
*/
void MetaData::setStreamDir(StreamDir stream_dir) {
	this->stream_dir = stream_dir;
}

void MetaData::setDataType(DataType data_type) {
	this->data_type = data_type;
}

void MetaData::setNumDim(NumDim num_dim) {
	this->num_dim = num_dim;
	this->data_shape.num_dim = num_dim;
}

void MetaData::setMemOrder(MemOrder mem_order) {
	this->mem_order = mem_order;
}

void MetaData::setDataSize(DataSize data_size) {
	assert(data_size.size() == num_dim.toInt());
	assert(all(data_size >= 1));

	data_shape.data_size = data_size;
}

void MetaData::setBlockSize(BlockSize block_size) {
	assert(block_size.size() == num_dim.toInt());
	assert(all(block_size >= 1));
	assert(not getDataSize().isNone());
	assert(all(getDataSize() >= block_size));

	data_shape.block_size = block_size;
	data_shape.num_block = (getDataSize() + getBlockSize() - 1) / getBlockSize();
}

void MetaData::setGroupSize(GroupSize group_size) {
	assert(group_size.size() == num_dim.toInt());
	assert(all(group_size >= 1));
	assert(not getBlockSize().isNone());
	assert(all(getBlockSize() >= group_size));

	data_shape.group_size = group_size;
	data_shape.num_group = (getBlockSize() + getGroupSize() - 1) / getGroupSize();
}

size_t MetaData::totalDataSize() const {
	return prod(static_cast<Array4<size_t>>(data_shape.data_size)) * data_type.sizeOf();
}

size_t MetaData::totalBlockSize() const {
	return prod(static_cast<Array4<size_t>>(data_shape.block_size)) * data_type.sizeOf();
}

size_t MetaData::totalGroupSize() const {
	return prod(static_cast<Array4<size_t>>(data_shape.group_size)) * data_type.sizeOf();
}

} } // namespace map::detail
