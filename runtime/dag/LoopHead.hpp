/**
 * @file	LoopHead.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_LOOPHEAD_HPP_
#define MAP_RUNTIME_DAG_LOOPHEAD_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Loop; //!< Forward declaration

struct LoopHead : public Node
{
	// Constructors & methods
	LoopHead(Loop *loop, Node *prev);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Loop* loop() const;
	Node* prev() const;
	Pattern pattern() const { return SPREAD; }

	// Variables
};

} } // namespace map::detail

#endif
