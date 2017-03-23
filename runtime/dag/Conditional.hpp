/**
 * @file	Conditional.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Local Conditional operation (i.e. cond ? left : right)
 *
 * TODO: tensorflow features select() and cond().
 * - select() is an element-wise conditional, like this op.
 * - cond() takes a scalar 'pred' and performs true branching
 * If we use a Zonal for 'pred', at runtime one side could be optimized away
 */

#ifndef MAP_RUNTIME_DAG_CONDITIONAL_HPP_
#define MAP_RUNTIME_DAG_CONDITIONAL_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Conditional : public Node
{
	// Internal declarations
	struct Key {
		Key(Conditional *node);
		bool operator==(const Key& k) const;
		Node *cond, *lprev, *rprev;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *cond, Node *lhs, Node *rhs);
	Node* clone(NodeList new_prev_list, NodeList new_back_list);

	// Constructors
	Conditional(const MetaData &meta, Node *cond, Node *lprev, Node *rprev);
	Conditional(const Conditional *other, NodeList new_prev_list, NodeList new_back_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& cond(); // Condition node
	Node* cond() const;
	//Node*& left(); // Left node
	Node* left() const;
	//Node*& right(); // Right node
	Node* right() const;
	Pattern pattern() const { return LOCAL; }

	// Variables
};

} } // namespace map::detail

#endif
