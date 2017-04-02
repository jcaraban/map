/**
 * @file	Switch.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_SWITCH_HPP_
#define MAP_RUNTIME_DAG_SWITCH_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Switch : public Node
{
	// Internal declarations
	struct Key {
		Key(Switch *node);
		bool operator==(const Key& k) const;
		Node *cond, *prev;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *cond, Node *prev);
	Node* clone(std::unordered_map<Node*,Node*> other_to_this);
	
	// Constructors
	Switch(const MetaData &meta, Node *cond, Node *prev);
	Switch(const Switch *other, std::unordered_map<Node*,Node*> other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* cond() const;
	Node* prev() const;
	Pattern pattern() const;

	const NodeList& nextList(bool true_false) const;

	// Variables
	NodeList next_true, next_false;
};

} } // namespace map::detail

#endif
