/**
 * @file	SpreadScan.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: remove it in favour of loop + focal, or find another use
 */

#ifndef MAP_RUNTIME_DAG_SPREADSCAN_HPP_
#define MAP_RUNTIME_DAG_SPREADSCAN_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct SpreadScan : public Node
{
	// Internal declarations
	struct Content {
		Content(SpreadScan *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Node *dir;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, Node *dir, ReductionType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	SpreadScan(const MetaData &meta, Node *prev, Node *dir, ReductionType type);
	SpreadScan(const SpreadScan *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Node* dir() const;
	Node* spread() const;
	Node* buffer() const;
	Node* stable() const;
	Pattern pattern() const { return SPREAD; }

	// Compute
	//void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	//void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);
	
	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
