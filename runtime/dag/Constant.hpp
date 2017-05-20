/**
 * @file	Constant.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Constant value (e.g. 0, 1.0f, 3.1415926535)
 */

#ifndef MAP_RUNTIME_DAG_CONSTANT_HPP_
#define MAP_RUNTIME_DAG_CONSTANT_HPP_

#include "Node.hpp"
 

namespace map { namespace detail {

struct Constant : public Node
{
	// Internal declarations
	struct Content {
		Content(Constant *node);
		bool operator==(const Content& k) const;
		NumDim num_dim;
		VariantType cnst;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(VariantType val, DataSize ds, DataType dt, MemOrder mo, BlockSize bs, GroupSize gs);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Constant(const MetaData &meta, VariantType val);
	Constant(const Constant *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;

	// Spatial
	Pattern pattern() const { return FREE; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;
	
	// Features
	bool isConstant() const { return true; }
	
	// Compute
	void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	VariantType cnst;
};

} } // namespace map::detail

#endif
