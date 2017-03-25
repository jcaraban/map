/**
 * @file    LoopAssembler.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Class in charge of assembling a Loop as it executes in Python
 */

#ifndef LOOPASSEMBLER_HPP_
#define LOOPASSEMBLER_HPP_

#include "dag/Node.hpp"


namespace map { namespace detail {

enum LoopMode { NORMAL_MODE, LOOP_START, LOOP_BODY, LOOP_AGAIN };

struct LoopStruct {
	NodeList prev; //!< Previous existing nodes used in the loop
	NodeList cond; //!< Nodes expressing the halt condition
	NodeList body; //!< Nodes composing the main loop body
	NodeList again; //!< Again the body, to find feedbacks
	NodeList feed_in; //!< Nodes that feedback into body (input side)
	NodeList feed_out; //!< (output side), repeteadly swap with (feed_in)
	Node *loop;
	NodeList head; //!< Head nodes created by Loop
	NodeList tail; //!< Tail nodes created by Loop
	NodeList oldpy;
	NodeList newpy;
};

struct LoopAssembler {
  // Constructors
	LoopAssembler(int loop_nested_limit);

  // Methods
	void clear();
	LoopMode mode();

	void addNode(Node *node, Node *orig);
	void digestion(bool start, bool body, bool again, bool end);
	void condition(Node *cond);
	Node* assemble();
	void updateVars(Node *node, Node ***agains, Node ***tails, int *num);

  // Vars
	LoopMode loop_mode; //!< Indicates if we are inside a (possibly nested) loop
	int loop_nested_limit; //!< Maximum levels of nesting loops inside each other
	int loop_level; //!< Tracks the loop nesting level, from 0 .. loop_nested_limit
	std::vector<LoopStruct> loop_struct; //!< Stores the ongoing (nested?) loop
};

} } // namespace map::detail

#endif
