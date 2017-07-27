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
	struct Content {
		Content(Binary *node);
		bool operator==(const Content& k) const;
		Node *lprev, *rprev;
		BinaryType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *lhs, Node *rhs, BinaryType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Binary(const MetaData &meta, Node *lprev, Node *rprev, BinaryType type);
	Binary(const Binary *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* left() const;
	Node* right() const;
	
	// Spatial
	Pattern pattern() const { return LOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	BinaryType type; //!< Enum corresponding to the type of binary operation / function
};

} } // namespace map::detail

#endif
