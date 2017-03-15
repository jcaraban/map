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
	struct Key {
		Key(FocalPercent *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Mask smask;
		PercentType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, const Mask &mask, PercentType type);
	Node* clone(NodeList new_prev_list);

	// Constructors
	FocalPercent(const MetaData &meta, Node *prev, const Mask &mask, PercentType type);
	FocalPercent(const FocalPercent *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Mask mask() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Variables
	Mask smask; //!< Static mask
	PercentType type;
};

} } // namespace map::detail

#endif
