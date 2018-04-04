/**
 * @file	Mask.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: now the mask size must be odd and the center is always 'ds/2'
 *
 * TODO: ponder changing the name to 1.Kernel 2.Filter 3.Matrix 4.Mask
 * TODO: consider moving iterSpace() out of Mask
 */

#ifndef MAP_UTIL_MASK_HPP_
#define MAP_UTIL_MASK_HPP_

#include "Array.hpp"
#include "VariantType.hpp"
#include <vector>


namespace map { namespace detail {

/*
 *
 */
struct Mask
{	
	Array<VariantType> array;
	DataType data_type;
	DataSize data_size;
	NumDim num_dim;
	//Coord center;

  public:
  	Mask();
  	Mask(DataSize data_size, VariantType scalar);
  	Mask(DataSize data_size, Array<VariantType> array);
	template <typename T> Mask(DataSize data_size, Array<T> array);
	DataType datatype() const;
	NumDim numdim() const;
	const DataSize& datasize() const;

	bool isNone() const;
	bool isZero() const;

	//VariantType operator[](int i) const;
	VariantType operator[](Coord c) const;
	bool operator==(const Mask &m) const;
	size_t hash() const;
	std::string signature() const;

	Mask invert() const;

	std::vector<Coord> cellSpace() const;
	std::vector<Coord> blockSpace(BlockSize bs) const;
};

Mask pipe(const Mask& lhs, const Mask& rhs);
Mask flat(const Mask& lhs, const Mask& rhs);

} } // namespace map::detail

#endif

#include "Mask.tpl"
