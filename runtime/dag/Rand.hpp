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
	struct Key {
		Key(Rand *node);
		bool operator==(const Key& k) const;
		Node *seed;
		// Should Key be defined by more things? e.g. num dim, mem order, data size
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *seed, DataType dt, MemOrder mo);
	static Node* Factory(VariantType seed, DataSize ds, DataType dt, MemOrder mo, BlockSize bs);

	// Constructors & methods
	Rand(const MetaData &meta, Node *seed);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& seed();
	Node* seed() const;
	Pattern pattern() const { return LOCAL; }

	// Variables
};

} } // namespace map::detail

#endif
