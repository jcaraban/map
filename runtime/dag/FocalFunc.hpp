/**
 * @file	FocalFunc.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Generic operation with static mask and configurable RedOP and BinOP
 *
 * TODO: a FocalFuncType should be added to allow more ops. than multiplication with the mask
 */

#ifndef MAP_RUNTIME_DAG_FOCALFUNC_HPP_
#define MAP_RUNTIME_DAG_FOCALFUNC_HPP_

#include "Node.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

struct FocalFunc : public Node
{
	// Internal declarations
	struct Key {
		Key(FocalFunc *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Mask smask;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, const Mask &mask, ReductionType type);
	Node* clone(std::unordered_map<Node*,Node*> other_to_this);

	// Constructors
	FocalFunc(const MetaData &meta, Node *prev, const Mask &mask, ReductionType type);
	FocalFunc(const FocalFunc *other, std::unordered_map<Node*,Node*> other_to_this);

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
	ReductionType type;
	//FocalFuncType btype;
};

} } // namespace map::detail

#endif
