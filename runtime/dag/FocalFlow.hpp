/**
 * @file	FocalFlow.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Flow-direction operation with static algorithm (e.g. D8, D16)
 *
 * TODO: is currently implemented with other elemental Local + Focal operations
 */

#ifndef MAP_RUNTIME_DAG_FOCALFLOW_HPP_
#define MAP_RUNTIME_DAG_FOCALFLOW_HPP_

#include "Node.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

struct FocalFlow : public Node
{
	// Internal declarations
	struct Key {
		Key(FocalFlow *node);
		bool operator==(const Key& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg);

	// Constructors & methods
	FocalFlow(const MetaData &meta, Node *prev);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Variables
};

} } // namespace map::detail

#endif
