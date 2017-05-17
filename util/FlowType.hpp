/**
 * @file	FlowType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: flow ACU is an iterative OP
 * Note: flow DIR is might be expressed with another primitive Focal OP
 */

#ifndef MAP_UTIL_FLOWTYPE_HPP_
#define MAP_UTIL_FLOWTYPE_HPP_

#include "VariantType.hpp"
#include <string>


namespace map { namespace detail {

// Enum

enum FlowEnum { NONE_FLOW, DIR, ACU, N_FLOW };

// Class

class FlowType {
	FlowEnum type;

  public:
  	FlowType();
  	FlowType(FlowEnum type);
  	FlowEnum get() const;

  	bool operator==(FlowType type) const;
  	bool operator!=(FlowType type) const;

	std::string toString() const;
	std::string code() const;
};

static_assert( std::is_standard_layout< FlowType >::value , "FlowType must be C compatible");

} } // namespace map::detail

#endif
