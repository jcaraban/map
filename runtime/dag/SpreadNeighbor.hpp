/**
 * @file	SpreadNeighbor.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal SpreadNeighbor operation with static coordinate
 */

#ifndef MAP_RUNTIME_DAG_SPREADNEIGHBOR_HPP_
#define MAP_RUNTIME_DAG_SPREADNEIGHBOR_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct SpreadNeighbor : public Node
{
	// Internal declarations
	struct Key {
		Key(SpreadNeighbor *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Node *dir;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, Node *dir, ReductionType type);
	Node* clone(NodeList new_prev_list);

	// Constructors
	SpreadNeighbor(const MetaData &meta, Node *prev, Node *dir, ReductionType type);
	SpreadNeighbor(const SpreadNeighbor *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Node* dir() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
