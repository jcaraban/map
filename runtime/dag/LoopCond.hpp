/**
 * @file	LoopCond.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: might become a general 'condition' node later, used for if-else too
 *		 in such case it is better to change 'Conditional' to another name
 */

#ifndef MAP_RUNTIME_DAG_LOOPCOND_HPP_
#define MAP_RUNTIME_DAG_LOOPCOND_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Loop; //!< Forward declaration

struct LoopCond : public Node
{
	// Internal declarations
	struct Key {
		Key(LoopCond *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Loop *loop;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	Node* clone(NodeList new_prev_list);

	// Constructors
	LoopCond(Loop *loop, Node *prev);
	LoopCond(const LoopCond *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Loop* loop() const;
	Node* prev() const;
	Pattern pattern() const { return SPREAD; }

	// Variables
	Loop *owner_loop;
};

} } // namespace map::detail

#endif
