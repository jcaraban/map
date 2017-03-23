/**
 * @file	BoundedNbh.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Neighbor operation with dynamic coordinate
 *
 * TODO: find a better name
 */

#ifndef MAP_RUNTIME_DAG_BOUNDEDNBH_HPP_
#define MAP_RUNTIME_DAG_BOUNDEDNBH_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct BoundedNbh : public Node
{
	// Internal declarations
	struct Key {
		Key(BoundedNbh *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Node *cx, *cy;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, Node *cx, Node *cy);
	Node* clone(NodeList new_prev_list, NodeList new_back_list);

	// Constructors
	BoundedNbh(const MetaData &meta, Node *prev, Node *cx, Node *cy);
	BoundedNbh(const BoundedNbh *other, NodeList new_prev_list, NodeList new_back_list);
	
	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	//Node* coord() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Variables
	//!< Dynamic neighbor direction is in prev_list
};

} } // namespace map::detail

#endif
