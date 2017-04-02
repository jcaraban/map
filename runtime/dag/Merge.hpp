/**
 * @file	Merge.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Merge operation, equivalent to PHI in dataflow
 */

#ifndef MAP_RUNTIME_DAG_MERGE_HPP_
#define MAP_RUNTIME_DAG_MERGE_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Merge : public Node
{
	// Internal declarations
	struct Key {
		Key(Merge *node);
		bool operator==(const Key& k) const;
		Node *lprev, *rprev;
		Pattern pat;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *lhs, Node *rhs);
	Node* clone(std::unordered_map<Node*,Node*> other_to_this);

	// Constructors
	Merge(const MetaData &meta, Node *lprev, Node *rprev);
	Merge(const Merge *other, std::unordered_map<Node*,Node*> other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& left(); // Left node
	Node* left() const;
	//Node*& right(); // Right node
	Node* right() const;
	Pattern pattern() const;

	// Variables
	// @ should we have a Merge <--> Switch twins link?
};

} } // namespace map::detail

#endif
