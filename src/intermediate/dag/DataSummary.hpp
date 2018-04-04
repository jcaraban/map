/**
 * @file	DataSummary.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a DataSummary operation, i.e. computes whole-data summaries
 */

#ifndef MAP_RUNTIME_DAG_DATASUMMARY_HPP_
#define MAP_RUNTIME_DAG_DATASUMMARY_HPP_

#include "../Node.hpp"


namespace map { namespace detail {

struct DataSummary : public Node
{
	// Internal declarations
	struct Content {
		Content(DataSummary *node);
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
	DataSummary(const MetaData &meta, Node *prev, ReductionType type);
	DataSummary(const DataSummary *other, const std::unordered_map<Node*,Node*> &other_to_this);

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
	ReductionType reductype() const { return type; }

	// Compute
	//void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
