/**
 * @file	RadialScan.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Radial Scan operation (i.e. sum,prod,min,max Radial to the borders from a starting point)
 */

#ifndef MAP_RUNTIME_DAG_RADIALSCAN_HPP_
#define MAP_RUNTIME_DAG_RADIALSCAN_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct RadialScan : public Node
{
	// Internal declarations
	struct Content {
		Content(RadialScan *node);
		bool operator==(const Content& k) const;
		Node *prev;
		ReductionType type;
		Coord start;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, ReductionType type, Coord start);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	RadialScan(const MetaData &meta, Node *prev, ReductionType type, Coord start);
	RadialScan(const RadialScan *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	
	// Spatial
	Pattern pattern() const { return RADIAL; }
	const Mask& inputReach(Coord coord) const;
	// const Mask& intraReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	ReductionType type;
	Coord start, start_block; //!< Starting coordinates
	Mask center, north, east, south, west;
};

} } // namespace map::detail

#endif
