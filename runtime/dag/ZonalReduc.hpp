/**
 * @file	ZonalReduc.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Reduction operation that reduces multi-dimensional data to one scalar
 */

#ifndef MAP_RUNTIME_DAG_ZONALREDUC_HPP_
#define MAP_RUNTIME_DAG_ZONALREDUC_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct ZonalReduc : public Node
{
	// Internal declarations
	struct Key {
		Key(ZonalReduc *node);
		bool operator==(const Key& k) const;
		Node *prev;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, ReductionType type);

	// Constructors & methods
	ZonalReduc(const MetaData &meta, Node *prev, ReductionType type);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Pattern pattern() const { return ZONAL; }

	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
