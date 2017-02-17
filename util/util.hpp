/**
 * @file	util.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Includes all utilities
 */

#ifndef MAP_UTIL_HPP_
#define MAP_UTIL_HPP_

#include "StreamDir.hpp"
#include "DataType.hpp"
#include "NumDim.hpp"
#include "MemOrder.hpp"

#include "Array.hpp"
#include "Array4.hpp"

#include "VariantType.hpp"
#include "UnaryType.hpp"
#include "BinaryType.hpp"
#include "ReductionType.hpp"
#include "DiversityType.hpp"
#include "PercentType.hpp"

#include "Mask.hpp"

#include "null.hpp"
#include "common.hpp"


namespace map { namespace detail {

enum TypeMem {PRIVATE, SHARED, LITERAL};
enum TypeVar {SCALAR, ARRAY, POINTER};

} } // namespace map::detail

#endif
