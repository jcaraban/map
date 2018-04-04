/**
 * @file	Mask.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: remove the exception code added to ARRA4 BINARY OP and handle the exceptions here
 */

#include "Mask.hpp"


namespace map { namespace detail {

namespace {
	void set(Mask &mask, Coord coord, VariantType var) {
		auto dt = mask.datatype();
		auto dim = mask.datasize();
		auto off = dim / 2;
		mask.array[proj(coord,dim,off)] = var.convert(dt);
	}
}

Mask::Mask() { }

Mask::Mask(DataSize data_size, VariantType scalar) {
	assert(not data_size.isNone());
	assert(all(data_size % 2)); // odd

	this->array = Array<VariantType>(prod(data_size),scalar);
	this->data_type = scalar.datatype();
	this->num_dim = DataSize2NumDim(data_size);
	this->data_size = data_size;
}

Mask::Mask(DataSize data_size, Array<VariantType> array) {
  	assert(prod(data_size) == array.size());
	assert(all(data_size % 2)); // odd
	assert(array.size() > 0);
	
	this->array = array;
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

bool Mask::isNone() const {
	//return array.isNone();
	assert(0);
}

bool Mask::isZero() const {
	return datasize().size() == 0;
}

//VariantType Mask::operator[](int i) const {
//	return array[i];
//}

VariantType Mask::operator[](Coord coord) const {
	auto dim = datasize();
	auto off = dim / 2;
	return array[proj(coord,dim,off)];
}

bool Mask::operator==(const Mask &m) const {
	if (data_type != m.data_type)
		return false;
	if (num_dim != m.num_dim)
		return false;
	if (any(data_size != m.data_size))
		return false;
	bool b = true;
	for (int i=0; i<array.size(); i++)
		b &= array[i].isEqual(m.array[i]);
	return b;
}

std::size_t Mask::hash() const {
	size_t h = 0;
	for (int i=0; i<array.size(); i++)
		h ^= array[i].hash();
	return h;
}

std::string Mask::signature() const {
	std::string sign = "";
	sign += numdim().toString();
	sign += datatype().toString();
	sign += to_string(datasize());
	for (int i=0; i<array.size(); i++)
		sign += array[i].toString();
	return sign;
}

Mask Mask::invert() const {
	const Mask &mask = *this; 
	Mask inver = *this;
	for (auto coord : mask.cellSpace()) {
		if (all(coord == 0))
			break; // stops in the middle
		inver[0-coord] = mask[coord];
	}
	return inver;
}

std::vector<Coord> Mask::cellSpace() const {
	Coord start = datasize() / -2;
	Coord stop = datasize() / 2 + 1;
	auto vec = iterSpace(start,stop);
	auto pred = [&](Coord coord){ return !(*this)[coord]; };
	vec.erase(std::remove_if(vec.begin(),vec.end(),pred),vec.end());
	return vec;
}

std::vector<Coord> Mask::blockSpace(BlockSize bs) const {
	auto rad = datasize() / 2;
	rad = (rad + bs - 1) / bs; // iceil
	Coord start = 0 - rad;
	Coord stop = rad + 1;
	return iterSpace(start,stop);
}

Mask pipe(const Mask& lhs, const Mask& rhs) {
	if (lhs.isZero())
		return rhs;
	if (rhs.isZero())
		return lhs;

	DataSize lds = lhs.datasize();
	DataSize rds = rhs.datasize();
	DataSize tds = (lds/2 + rds/2)*2 + 1;
	Mask out = Mask(tds,false);

	for (auto lcoord : lhs.cellSpace()) {
		set(out,lcoord,true);
		for (auto rcoord : rhs.cellSpace()) {
			set(out,lcoord+rcoord,true);
		}
	}

	return out;
}

Mask flat(const Mask& lhs, const Mask& rhs) {
	if (lhs.isZero())
		return rhs;
	if (rhs.isZero())
		return lhs;

	DataSize lds = lhs.datasize();
	DataSize rds = rhs.datasize();
	DataSize tds = max(lds,rds);
	Mask out = Mask(tds,false);

	for (auto lcoord : lhs.cellSpace()) {
		set(out,lcoord,true);
	}

	for (auto rcoord : rhs.cellSpace()) {
		set(out,rcoord,true);
	}

	return out;
}

} } // namespace detail, map
