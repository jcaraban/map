/**
 * @file	Block0.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: reduce constructors to just one ?
 */

#include "Block0.hpp"


namespace map { namespace detail {

Block0::Block0()
	: Block()
{ }

Block0::Block0(Key key, int dep)
	: Block(key,dep)
{ }

Block0::~Block0() { }

HoldType Block0::holdtype() const {
	return HOLD_0;
}

bool Block0::isFixed() const {
	return false; // @
}

} } // namespace map::detail
