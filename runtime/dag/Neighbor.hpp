/**
 * @file	Neighbor.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Neighbor operation with static coordinate
 */

#ifndef MAP_RUNTIME_DAG_NEIGHBOR_HPP_
#define MAP_RUNTIME_DAG_NEIGHBOR_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Neighbor : public Node
{
	// Internal declarations
	struct Content {
		Content(Neighbor *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Coord scoord;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, const Coord &coord);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Neighbor(const MetaData &meta, Node *prev, const Coord &coord);
	Neighbor(const Neighbor *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Coord coord() const;

	// Spatial
	Pattern pattern() const { return FOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	//void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	Coord scoord; //!< Static neighbor direction
};

} } // namespace map::detail

#endif
