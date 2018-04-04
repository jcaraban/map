/**
 * @file    LoopAssembler.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Class in charge of assembling a Loop as it executes in Python
 */

#ifndef MAP_RUNTIME_LOOPASSEMBLER_HPP_
#define MAP_RUNTIME_LOOPASSEMBLER_HPP_

#include "../intermediate/Node.hpp"
#include "../intermediate/dag/Empty.hpp"
#include "../intermediate/dag/LoopCond.hpp"


namespace map { namespace detail {

enum LoopMode { NORMAL_MODE, LOOP_START, LOOP_BODY, LOOP_AGAIN };

struct LoopStruct {
	NodeList prev; //!< Previous existing nodes used in the loop
	NodeList cond; //!< Nodes computing the halt condition
	NodeList body; //!< Nodes composing the main loop body
	NodeList again; //!< Again the body, to find circulating nodes
	NodeList circ_in; //!< Nodes whose data circulates in the loop (input side, € 'prev'),
	NodeList circ_out; //!< (output side, € 'body'), the 'circ' lists match in size and order
	NodeList invar_in;
	NodeList invar_out;
	
	LoopCond *loop;
	HeadList head; //!< Head nodes created by Loop
	TailList tail; //!< Tail nodes created by Loop
	MergeList merge; //!< Merge nodes created by Loop
	SwitchList switc; //!< Switch nodes created by Loop
	NodeList empty; //!< Empty nodes created by Loop
	NodeList iden; //!< Identity nodes created bt Loop

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
	void assemble();
	void updateVars(Node *node, Node ***agains, Node ***tails, int *num);

  //
	void extract();
	void compose();

  // Vars
	LoopMode loop_mode; //!< Indicates if we are inside a (possibly nested) loop
	int loop_nested_limit; //!< Maximum levels of nesting loops inside each other
	int loop_level; //!< Tracks the loop nesting level, from 0 .. loop_nested_limit
	std::vector<LoopStruct> loop_struct; //!< Stores the ongoing (nested?) loop
};

} } // namespace map::detail

#endif
