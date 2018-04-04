/**
 * @file	Identity.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing the Identity function, which outputs its input unchanged
 */

#ifndef MAP_RUNTIME_DAG_IDENTITY_HPP_
#define MAP_RUNTIME_DAG_IDENTITY_HPP_

#include "../Node.hpp"


namespace map { namespace detail {

struct Identity : public Node
{
	// Internal declarations
	struct Content {
		Content(Identity *node);
		bool operator==(const Content& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Identity(const MetaData &meta, Node *prev);
	Identity(const Identity *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	
	// Features
	bool canForward() const { return true; };
	
	// Spatial
	Pattern pattern() const { return LOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;
	
	// Compute
	void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
};

} } // namespace map::detail

#endif
