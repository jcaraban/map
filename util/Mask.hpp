/**
 * @file	Mask.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: ponder changing the name to 1.Kernel 2.Filter 3.Matrix 4.Mask
 *       Kernel is what ESRI call this, since Filter is an operation
 */

#ifndef MAP_UTIL_MASK_HPP_
#define MAP_UTIL_MASK_HPP_

#include "Array.hpp"
#include "VariantType.hpp"


namespace map { namespace detail {

struct Mask
{	
	Array<VariantType> mask;
	DataType data_type;
	DataSize data_size;
	NumDim num_dim;

  public:
  	Mask();
  	Mask(DataSize data_size, Array<VariantType> array);
	template <typename T> Mask(DataSize data_size, Array<T> array);
	DataType datatype() const;
	NumDim numdim() const;
	const DataSize& datasize() const;

	VariantType operator[](int i) const;
	bool operator==(const Mask &m) const;
	size_t hash() const;
	std::string signature() const;
};

} } // namespace map::detail

#endif

#include "Mask.tpl"
