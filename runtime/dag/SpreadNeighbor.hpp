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
	static Node* Factory(Node *arg, Node *dir, ReductionType type);

	// Constructors & methods
	SpreadNeighbor(const MetaData &meta, Node *prev, Node *dir, ReductionType type);
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
