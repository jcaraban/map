/**
 * @file    NumDim.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: maybe add static toInt() function, to enable loop unrolling
 */

#ifndef MAP_UTIL_NUMDIM_HPP_
#define MAP_UTIL_NUMDIM_HPP_

#include <string>


namespace map { namespace detail {

// Enum

enum NumDimEnum : int { NONE_NUMDIM=0x00, TIME=0x01, D0=0x02, D1=0x04, D2=0x06, D3=0x08, N_NUMDIM=0x0A };

// Class

class NumDim {
	NumDimEnum dim;

  public:
	NumDim();
	NumDim(NumDimEnum dim);
	NumDimEnum get() const;

	bool operator==(NumDim dim) const;
	bool operator!=(NumDim dim) const;
	bool is(NumDim dim) const;

	int toInt() const;
	std::string toString() const;
};

// Util

constexpr NumDimEnum operator+(const NumDimEnum& lhs, const NumDimEnum& rhs) {
	return static_cast<NumDimEnum>( static_cast<int>(lhs) | static_cast<int>(rhs) );
}

} } // namespace map::detail

#endif

#include "NumDim.tpl"
