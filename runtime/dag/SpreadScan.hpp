/**
 * @file	SpreadScan.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_SPREADSCAN_HPP_
#define MAP_RUNTIME_DAG_SPREADSCAN_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct SpreadScan : public Node
{
	// Internal declarations
	struct Key {
		Key(SpreadScan *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Node *dir;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, Node *dir, ReductionType type);
	Node* clone(NodeList new_prev_list);

	// Constructors
	SpreadScan(const MetaData &meta, Node *prev, Node *dir, ReductionType type);
	SpreadScan(const SpreadScan *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	//Node*& dir();
	Node* dir() const;
	//Node*& spread();
	Node* spread() const;
	//Node*& buffer();
	Node* buffer() const;
	//Node*& stable();
	Node* stable() const;
	Pattern pattern() const { return SPREAD; }

	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
