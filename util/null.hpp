/**
 * @file	null.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * NULL-compatibility is basically non-existent
 *
 * TODO: come up with a design for the 'null' functionalities
 */

#ifndef MAP_UTIL_NULL_HPP_
#define MAP_UTIL_NULL_HPP_

#include "DataType.hpp"
#include <cmath>
#include <limits>


namespace map { namespace detail {

template <DataTypeEnum T> struct null {
	static const Ctype<T> value = std::numeric_limits<Ctype<T>>::max;
};
template <> struct null< F32 > {
	constexpr static const Ctype<F32> value = std::numeric_limits<Ctype<F32>>::quiet_NaN();
};
template <> struct null< F64 > {
	constexpr static const Ctype<F64> value = std::numeric_limits<Ctype<F64>>::quiet_NaN();
};

inline bool isNull(float value) { return std::isnan(value); }
inline bool isNull(double value) { return std::isnan(value); }
inline float& setNull(float& value) { return value = null<F32>::value; }
inline double& setNull(double& value) { return value = null<F64>::value; }

} } // namespace map::detail

#endif
