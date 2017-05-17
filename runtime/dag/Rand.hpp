/**
 * @file	Rand.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Random number generator
 */

#ifndef MAP_RUNTIME_DAG_RAND_HPP_
#define MAP_RUNTIME_DAG_RAND_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Rand : public Node
{
	// Internal declarations
	struct Content {
		Content(Rand *node);
		bool operator==(const Content& k) const;
		Node *seed;
		// Should Content be defined by more things? e.g. num dim, mem order, data size
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *seed, DataType dt, MemOrder mo);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Rand(const MetaData &meta, Node *seed);
	Rand(const Rand *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* seed() const;

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
