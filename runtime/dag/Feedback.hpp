/**
 * @file	Feedback.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_FEEDBACK_HPP_
#define MAP_RUNTIME_DAG_FEEDBACK_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Loop; //!< Forward declaration
struct LoopHead; //!< Forward declaration

struct Feedback : public Node
{
	// Constructors & methods
	Feedback(Loop *loop, LoopHead *prev);
	Feedback(Loop *loop, Feedback *feed_in, Node *prev);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Loop* loop() const;
	Node* prev() const;
	Pattern pattern() const { return SPREAD; }

	//const NodeList& prevList() const;
	//const NodeList& nextList() const;

	// Variables
	Feedback *twin; // linked feedback (i.e. feed_in <--> feed_out)
	//NodeList prev_both, next_both;
};

} } // namespace map::detail

#endif
