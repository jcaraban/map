/**
 * @file	FocalPercent.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Percent-age/tile operation with static mask
 *
 * TODO: @ could be implemented as FocalFunc with EQ/GT and SUM
 */

#ifndef MAP_RUNTIME_DAG_FOCALPERCENT_HPP_
#define MAP_RUNTIME_DAG_FOCALPERCENT_HPP_

#include "Node.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

struct FocalPercent : public Node
{
	// Internal declarations
	struct Content {
		Content(FocalPercent *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Mask smask;
		PercentType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, const Mask &mask, PercentType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	FocalPercent(const MetaData &meta, Node *prev, const Mask &mask, PercentType type);
	FocalPercent(const FocalPercent *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Mask mask() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	//void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	Mask smask; //!< Static mask
	PercentType type;
};

} } // namespace map::detail

#endif
