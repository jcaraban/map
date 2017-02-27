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
	// Constructors & methods
	LoopCond(Loop *loop, Node *prev);
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
