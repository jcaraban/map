/**
 * @file	Summary.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node that manually computes the statistics. Returns the input node, now including the statistics
 *
 * TODO: remove STATS from the patterns
 */

#ifndef MAP_RUNTIME_DAG_SUMMARY_HPP_
#define MAP_RUNTIME_DAG_SUMMARY_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Summary : public Node
{
	// Internal declarations
	struct Content {
		Content(Summary *node);
		bool operator==(const Content& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, Node *min, Node *max, Node *mean, Node *std);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Summary(const MetaData &meta, Node *prev, Node *min, Node *max, Node *mean, Node *std);
	Summary(const Summary *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Node* min() const;
	Node* max() const;
	Node* mean() const;
	Node* std() const;

	// Features
	bool canForward() const { return true; };

	// Spatial
	Pattern pattern() const { return STATS; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	//void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);
	
	// Variables
};

} } // namespace map::detail

#endif
