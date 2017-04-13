/**
 * @file	SpreadNeighbor.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal SpreadNeighbor operation with static coordinate
 */

#ifndef MAP_RUNTIME_DAG_SPREADNEIGHBOR_HPP_
#define MAP_RUNTIME_DAG_SPREADNEIGHBOR_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct SpreadNeighbor : public Node
{
	// Internal declarations
	struct Content {
		Content(SpreadNeighbor *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Node *dir;
		ReductionType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, Node *dir, ReductionType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	SpreadNeighbor(const MetaData &meta, Node *prev, Node *dir, ReductionType type);
	SpreadNeighbor(const SpreadNeighbor *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Node* dir() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	//void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);
	
	// Variables
	ReductionType type;
};

} } // namespace map::detail

#endif
