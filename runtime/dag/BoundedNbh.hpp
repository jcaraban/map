/**
 * @file	BoundedNbh.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Neighbor operation with dynamic coordinate
 * The coordinate is 'bounded' to a certain distance or neighborhood
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
	struct Content {
		Content(BoundedNbh *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Node *cx, *cy;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, Node *cx, Node *cy);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	BoundedNbh(const MetaData &meta, Node *prev, Node *cx, Node *cy);
	BoundedNbh(const BoundedNbh *other, const std::unordered_map<Node*,Node*> &other_to_this);
	
	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Node* coordx() const;
	Node* coordy() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	//!< Dynamic neighbor direction is in prev_list
};

} } // namespace map::detail

#endif
