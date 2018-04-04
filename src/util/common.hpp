/**
 * @file	common.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Common utilities related to the Nodes, Groups, Task, containers, etc
 */

#ifndef MAP_UTIL_COMMON_HPP_
#define MAP_UTIL_COMMON_HPP_

#include <type_traits>
#include <algorithm>


namespace map { namespace detail {

/*
 * Returns if 'next' node / group / task is next of 'base' (with any number of elements in between)
 */
template <typename T>
bool is_next(T *base, T *next) {
	for (auto i : base->nextList())
		if (i == next || is_next(i,next))
			return true;
	return false;
}

/*
 * Returns if 'prev' node / group / task is prev of 'base' (with any number of elements in between)
 */
template <typename T>
bool is_prev(T *base, T *prev) {
	for (auto i : base->prevList())
		if (i == prev || is_prev(i,prev))
			return true;
	return false;
}

} } // namespace map::detail

#endif
