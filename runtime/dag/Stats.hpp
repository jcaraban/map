/**
 * @file	Stats.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node that manually computes the statistics. Returns the input node, now including the statistics
 *
 * TODO: @@ refactor this functionality, perhaps decompose it in multiple operations, think about it...
 * TODO: remove STATS from the patterns
 */

#ifndef MAP_RUNTIME_DAG_STATS_HPP_
#define MAP_RUNTIME_DAG_STATS_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Stats : public Node
{
	// Internal declarations
	struct Content {
		Content(Stats *node);
		bool operator==(const Content& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Stats(const MetaData &meta, Node *prev);
	Stats(const Stats *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Node* max() const;
	Node* min() const;
	Pattern pattern() const { return STATS; }

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);
	
	// Variables
};

} } // namespace map::detail

#endif
