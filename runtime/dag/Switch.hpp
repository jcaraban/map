/**
 * @file	Switch.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Switch operation, somewhat equivalent to a branch
 *
 * TODO: add a Merge <--> Switch twins link to mark the structured flow ?
 */

#ifndef MAP_RUNTIME_DAG_SWITCH_HPP_
#define MAP_RUNTIME_DAG_SWITCH_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Switch : public Node
{
	// Internal declarations
	struct Content {
		Content(Switch *node);
		bool operator==(const Content& k) const;
		Node *cond, *prev;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *cond, Node *prev);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);
	
	// Constructors
	Switch(const MetaData &meta, Node *cond, Node *prev);
	Switch(const Switch *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* cond() const;
	Node* prev() const;

	const NodeList& trueList() const;
	const NodeList& falseList() const;
	void addTrue(Node *node);
	void addFalse(Node *node);

	// Spatial
	Pattern pattern() const { return SWITCH; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	NodeList next_true, next_false;
};

} } // namespace map::detail

#endif
