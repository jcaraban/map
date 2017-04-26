/**
 * @file	FocalFunc.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Generic operation with static mask and configurable RedOP and BinOP
 *
 * TODO: a FocalFuncType should be added to allow more ops. than multiplication with the mask
 *       http://desktop.arcgis.com/en/arcmap/latest/manage-data/raster-and-images/statistics-function.htm
 */

#ifndef MAP_RUNTIME_DAG_FOCALFUNC_HPP_
#define MAP_RUNTIME_DAG_FOCALFUNC_HPP_

#include "Node.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

struct FocalFunc : public Node
{
	// Internal declarations
	struct Content {
		Content(FocalFunc *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Mask smask;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, const Mask &mask, ReductionType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	FocalFunc(const MetaData &meta, Node *prev, const Mask &mask, ReductionType type);
	FocalFunc(const FocalFunc *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Mask mask() const;

	// Spatial
	Pattern pattern() const { return FOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	Mask smask; //!< Static mask
	ReductionType type;
	//FocalFuncType ftype;
};

} } // namespace map::detail

#endif
