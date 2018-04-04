/**
 * @file	DataStats.tpl
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_UTIL_DATASTATS_TPL_
#define MAP_UTIL_DATASTATS_TPL_

#include <limits>


namespace map { namespace detail {

template <DataTypeEnum T>
CellStats defaultStats1() {
	CellStats cell;

	Ctype<F64> _min = std::numeric_limits<Ctype<T>>::min();
	Ctype<F64> _max = std::numeric_limits<Ctype<T>>::max();
	Ctype<F64> _mean = (_min + _max) / 2.0; // mid point of range
	Ctype<F64> _std = (_max - _min) / 4.0; // range rule of thumb

	cell.active = true;
	cell.data_type = T;
	cell.min = VariantType(_min,T);
	cell.max = VariantType(_max,T);
	cell.mean = VariantType(_mean,T);
	cell.std = VariantType(_std,T);

	return cell;
}

} } // namespace detail, map

#endif
