/**
 * @file	Neighbor.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Neighbor operation with static coordinate
 */

#ifndef MAP_RUNTIME_DAG_NEIGHBOR_HPP_
#define MAP_RUNTIME_DAG_NEIGHBOR_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Neighbor : public Node
{
	// Internal declarations
	struct Key {
		Key(Neighbor *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Coord scoord;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, const Coord &coord);
	Node* clone(NodeList new_prev_list);

	// Constructors
	Neighbor(const MetaData &meta, Node *prev, const Coord &coord);
	Neighbor(const Neighbor *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Coord coord() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Variables
	Coord scoord; //!< Static neighbor direction
};

} } // namespace map::detail

#endif
