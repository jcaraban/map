/**
 * @file	Mask.tpl
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_UTIL_MASK_TPL_
#define MAP_UTIL_MASK_TPL_

#include <cassert>


namespace map { namespace detail {

template <typename T> Mask::Mask(DataSize data_size, Array<T> array) {
  	assert(prod(data_size) == array.size());
	assert(array.size() > 0);
	
	//array = Array<VariantType>(array.size());
	for (int i=0; i<array.size(); i++)
		this->array[i] = VariantType(array[i]);

	this->data_type = Dtype<T>;
	this->num_dim = DataSize2NumDim(data_size);
	this->data_size = data_size;
}

} } // namespace map::detail

#endif
