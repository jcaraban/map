/**
 * @file	Key.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: can the hash function be tuned ?
 */

#include "Key.hpp"
#include "Runtime.hpp"
#include <algorithm>


namespace map { namespace detail {


Key::Key()
	: node(nullptr)
	, coord()
	, iter(0)
{ }

Key::Key(Node *node, Coord coord, size_t iter)
	: node(node)
	, coord(coord)
	, iter(iter)
{ }

bool Key::operator==(const Key& k) const {
	return (node==k.node && coord_equal()(coord,k.coord) && iter==k.iter);
}

std::size_t key_hash::operator()(const Key &k) const {
	return std::hash<Node*>()(k.node) ^ coord_hash()(k.coord) ^ std::hash<int>()(k.iter);
}

} } // namespace map::detail
