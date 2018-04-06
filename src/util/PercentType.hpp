/**
 * @file	PercentType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_UTIL_PERCENTTYPE_HPP_
#define MAP_UTIL_PERCENTTYPE_HPP_

#include "VariantType.hpp"
#include <string>


namespace map { namespace detail {

// Enum

enum PercentEnum : int {
	NONE_PERCENT, AGE, ILE, N_PERCENT
};

// Class

class PercentType {
	PercentEnum type;

  public:
  	PercentType();
  	PercentType(PercentEnum type);
  	PercentEnum get() const;

  	bool operator==(PercentType type) const;
  	bool operator!=(PercentType type) const;

	std::string toString() const;
	std::string code() const;
};

static_assert( std::is_standard_layout< PercentType >::value , "PercentType must be C compatible");

} } // namespace map::detail

#endif
