/**
 * @file	Mask.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Mask.hpp"


namespace map { namespace detail {

Mask::Mask() { }

Mask::Mask(DataSize data_size, Array<VariantType> array) {
  	assert(prod(data_size) == array.size());
	assert(array.size() > 0);
	
	this->mask = array;
	this->data_type = array[0].datatype();
	this->num_dim = DataSize2NumDim(data_size);
	this->data_size = data_size;
}

DataType Mask::datatype() const {
	return data_type;
}

NumDim Mask::numdim() const {
	return num_dim;
}

const DataSize& Mask::datasize() const {
	return data_size;
}

VariantType Mask::operator[](int i) const {
	return mask[i];
}

bool Mask::operator==(const Mask &m) const {
	if (data_type != m.data_type)
		return false;
	if (num_dim != m.num_dim)
		return false;
	if (any(data_size != m.data_size))
		return false;
	bool b = true;
	for (int i=0; i<mask.size(); i++)
		b &= mask[i].isEqual(m.mask[i]);
	return b;
}

std::size_t Mask::hash() const {
	size_t h = 0;
	for (int i=0; i<mask.size(); i++)
		h ^= mask[i].hash();
	return h;
}

std::string Mask::signature() const {
	std::string sign = "";
	sign += numdim().toString();
	sign += datatype().toString();
	sign += to_string(datasize());
	for (int i=0; i<mask.size(); i++)
		sign += mask[i].toString();
	return sign;
}

} } // namespace detail, map
