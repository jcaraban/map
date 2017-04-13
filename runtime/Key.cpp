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
{ }

Key::Key(Node *node, Coord coord)
	: node(node)
	, coord(coord)
{ }

bool Key::operator==(const Key& k) const {
	return (node==k.node && all(coord==k.coord));
}

std::size_t key_hash::operator()(const Key &k) const {
	std::size_t h = std::hash<Node*>()(k.node);
	for (int i=0; i<k.coord.size(); i++)
		h ^= std::hash<int>()(k.coord[i]);
	return h;
}

} } // namespace map::detail
