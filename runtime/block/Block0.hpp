/**
 * @file	Block0.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_RUNTIME_BLOCK_0_HPP_
#define MAP_RUNTIME_BLOCK_0_HPP_

#include "Block.hpp"


namespace map { namespace detail {

/*
 * @class Block0
 */
struct Block0 : public Block
{
  // Constructors
	Block0();
	Block0(Key key, int dep);
	~Block0() override;

  // Getters
	HoldType holdtype() const override;

  // Methods
	bool isFixed() const override; // @

  // Variables
};

} } // namespace map::detail

#endif
