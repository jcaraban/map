/**
 * @file	GroupSummary.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Summary operation, i.e. compute and collect per-block statistics of the data
 */

#ifndef MAP_RUNTIME_DAG_GROUPSUMMARY_HPP_
#define MAP_RUNTIME_DAG_GROUPSUMMARY_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct GroupSummary : public Node
{
	// Internal declarations
	struct Content {
		Content(GroupSummary *node);
		bool operator==(const Content& k) const;
		Node *prev;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, ReductionType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	GroupSummary(const MetaData &meta, Node *prev, ReductionType type);
	GroupSummary(const GroupSummary *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;

	// Spatial
	Pattern pattern() const { return STATS; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Features
	bool isReduction() const { return true; } // @@
	ReductionType reductype() const { return type; }
	
	// Compute
	//void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
