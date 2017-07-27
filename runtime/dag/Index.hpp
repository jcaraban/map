/**
 * @file	Index.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing the raster cells index for one specific dimension
 */

#ifndef MAP_RUNTIME_DAG_INDEX_HPP_
#define MAP_RUNTIME_DAG_INDEX_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Index : public Node
{
	// Internal declarations
	struct Content {
		Content(Index *node);
		bool operator==(const Content& k) const;
		NumDim dim;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(DataSize ds, NumDim dim, MemOrder mo, BlockSize bs, GroupSize gs);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Index(const MetaData &meta, NumDim dim);
	Index(const Index *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;

	// Spatial
	Pattern pattern() const { return FREE; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;
	
	// Compute
	void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	NumDim dim;
};

} } // namespace map::detail

#endif
