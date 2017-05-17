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

enum HoldType { NONE_HOLD, HOLD_0=0x1, HOLD_1=0x2, /*HOLD_2=0x4,*/ HOLD_N=0x8, N_HOLD };
enum DependType { DEPEND_UNKNOWN = -1, DEPEND_ZERO = 0 };

//typedef std::vector<Key> Depend;
typedef int Depend;

typedef std::vector<std::tuple<Key,HoldType,Depend>> KeyList;

} } // namespace map::detail

#endif
