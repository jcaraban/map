/**
 * @file	BoundedNeighbor.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Neighbor operation with dynamic coordinate
 * The coordinate is 'bounded' to a certain distance or neighborhood
 *
 * TODO: needs to know the worst-case neighborhood size
 * TODO: merge it with Neighbor ? Would need a 'tuple' node for the coords
 */

#ifndef MAP_RUNTIME_DAG_BOUNDEDNBH_HPP_
#define MAP_RUNTIME_DAG_BOUNDEDNBH_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct BoundedNeighbor : public Node
{
	// Internal declarations
	struct Content {
		Content(BoundedNeighbor *node);
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
	BoundedNeighbor(const MetaData &meta, Node *prev, Node *cx, Node *cy);
	BoundedNeighbor(const BoundedNeighbor *other, const std::unordered_map<Node*,Node*> &other_to_this);
	
	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Node* coordx() const;
	Node* coordy() const;
	
	// Spatial
	Pattern pattern() const { return FOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	//void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	//!< Dynamic neighbor direction is in prev_list
};

} } // namespace map::detail

#endif
