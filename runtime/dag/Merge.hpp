/**
 * @file	Merge.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Merge operation, somewhat equivalent to PHI
 *
 * TODO: add a Merge <--> Switch twins link to mark the structured flow ?
 */

#ifndef MAP_RUNTIME_DAG_MERGE_HPP_
#define MAP_RUNTIME_DAG_MERGE_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct MergeLoopFlag { };
struct MergeIfFlag { };

struct Merge : public Node
{
	// Internal declarations
	struct Content {
		Content(Merge *node);
		bool operator==(const Content& k) const;
		Node *lprev, *rprev;
		Pattern pat;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *lhs, Node *rhs, MergeLoopFlag flag);
	//static Node* Factory(Node *lhs, Node *rhs, MergeIfFlag flag);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Merge(const MetaData &meta, Node *lprev, Node *rprev, MergeLoopFlag not_used);
	Merge(const MetaData &meta, Node *lprev, Node *rprev, MergeIfFlag not_used);
	Merge(const Merge *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* left() const;
	Node* right() const;

	// Features
	bool canForward() const { return true; };

	// Spatial
	Pattern pattern() const { return MERGE; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
};

} } // namespace map::detail

#endif
