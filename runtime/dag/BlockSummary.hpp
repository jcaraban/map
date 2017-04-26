/**
 * @file	BlockSummary.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Summary operation, i.e. compute and collect per-block statistics of the data
 */

#ifndef MAP_RUNTIME_DAG_BLOCKSUMMARY_HPP_
#define MAP_RUNTIME_DAG_BLOCKSUMMARY_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct BlockSummary : public Node
{
	// Internal declarations
	struct Content {
		Content(BlockSummary *node);
		bool operator==(const Content& k) const;
		Node *prev;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, ReductionType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	BlockSummary(const MetaData &meta, Node *prev, ReductionType type);
	BlockSummary(const BlockSummary *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;

	// Spatial
	Pattern pattern() const { return ZONAL+STATS; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
