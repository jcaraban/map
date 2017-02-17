/**
 * @file	Binary.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Local Binary operation (e.g. +,-,*,/,==,&,<<,max,pow,atan2)
 */

#ifndef MAP_RUNTIME_DAG_BINARY_HPP_
#define MAP_RUNTIME_DAG_BINARY_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Binary : public Node
{
	// Internal declarations
	struct Key {
		Key(Binary *node);
		bool operator==(const Key& k) const;
		Node *lprev, *rprev;
		BinaryType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *lhs, Node *rhs, BinaryType type);

	// Constructors & methods
	Binary(const MetaData &meta, Node *lprev, Node *rprev, BinaryType type);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& left(); // Left node
	Node* left() const;
	//Node*& right(); // Right node
	Node* right() const;
	Pattern pattern() const { return LOCAL; }

	// Variables
	BinaryType type; //!< Enum corresponding to the type of binary operation / function
};

} } // namespace map::detail

#endif
