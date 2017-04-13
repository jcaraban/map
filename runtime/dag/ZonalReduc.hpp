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
	struct Content {
		Content(ZonalReduc *node);
		bool operator==(const Content& k) const;
		Node *prev;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, ReductionType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	ZonalReduc(const MetaData &meta, Node *prev, ReductionType type);
	ZonalReduc(const ZonalReduc *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Pattern pattern() const { return ZONAL; }

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
