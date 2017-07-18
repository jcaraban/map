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

	auto _min = std::numeric_limits<Ctype<T>>::min();
	auto _max = std::numeric_limits<Ctype<T>>::max();

	cell.active = true;
	cell.data_type = T;
	cell.min = _min;
	cell.max = _max;
	cell.mean = (_min + _max) / 2; // mid point of range
	cell.std = (_max - _min) / 4; // range rule of thumb

	return cell;
}

} } // namespace detail, map

#endif
