/**
 * @file	RadialScan.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Radial Scan operation (i.e. sum,prod,min,max radiating to the borders from a starting point)
 */

#ifndef MAP_RUNTIME_DAG_RADIALSCAN_HPP_
#define MAP_RUNTIME_DAG_RADIALSCAN_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct RadialScan : public Node
{
	// Internal declarations
	struct Key {
		Key(RadialScan *node);
		bool operator==(const Key& k) const;
		Node *prev;
		ReductionType type;
		Coord start;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, ReductionType type, Coord start);

	// Constructors & methods
	RadialScan(const MetaData &meta, Node *prev, ReductionType type, Coord start);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Pattern pattern() const { return RADIAL; }

	// Variables
	ReductionType type;
	Coord start; //!< Starting coordinates
};

} } // namespace map::detail

#endif
