/**
 * @file	Stats.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node that manually computes the statistics. Returns the input node, now including the statistics
 */

#ifndef MAP_RUNTIME_DAG_STATS_HPP_
#define MAP_RUNTIME_DAG_STATS_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Stats : public Node
{
	// Internal declarations
	struct Key {
		Key(Stats *node);
		bool operator==(const Key& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg);

	// Constructors & methods
	Stats(const MetaData &meta, Node *prev);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	//Node*& max();
	Node* max() const;
	//Node*& min();
	Node* min() const;
	Pattern pattern() const { return SPECIAL; }

	// Variables
};

} } // namespace map::detail

#endif
