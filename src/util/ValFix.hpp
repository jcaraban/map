/**
 * @file	ValFix.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: rethink ValFix, shall it be composed? splited like in Block::? etc.
 */

#ifndef MAP_UTIL_VALFIX_HPP_
#define MAP_UTIL_VALFIX_HPP_

#include "VariantType.hpp"
#include "../file/DataStats.hpp"
#include <string>


namespace map { namespace detail {

/*
 *
 */
struct ValFix {
	VariantType value, min, max, mean, std;
	bool fixed, active;

	ValFix();
	ValFix(VariantType val);
	//ValFix(CellStats sta);

	ValFix(VariantType val, bool fixed, CellStats sta);

	CellStats stats() const;
};

} } // namespace map::detail

#endif
