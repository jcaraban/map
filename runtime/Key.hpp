/**
 * @file	Key.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_KEY_HPP_
#define MAP_RUNTIME_KEY_HPP_

#include "../util/Array4.hpp"


namespace map { namespace detail {

class Node; // forward declaration

/*
 *
 */
struct Key {
	Node *node; //!< Node describing the operation
	Coord coord; //!< Spatial coordinate of the OP
	
	Key();
	Key(Node *node, Coord coord);
	bool operator==(const Key& k) const;
};

struct key_hash {
	std::size_t operator()(const Key& k) const;
};

} } // namespace map::detail

#endif
