/**
 * @file	Identity.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a type Identity to another data type (e.g. (int)x, (float)y)
 */

#ifndef MAP_RUNTIME_DAG_IDENTITY_HPP_
#define MAP_RUNTIME_DAG_IDENTITY_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Identity : public Node
{
	// Internal declarations
	struct Key {
		Key(Identity *node);
		bool operator==(const Key& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *prev);
	Node* clone(std::unordered_map<Node*,Node*> other_to_this);

	// Constructors
	Identity(const MetaData &meta, Node *prev);
	Identity(const Identity *other, std::unordered_map<Node*,Node*> other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Pattern pattern() const { return LOCAL; }

	// Variables
};

} } // namespace map::detail

#endif
