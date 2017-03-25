/**
 * @file    LoopAssembler.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: is_included is "expensive", it can be avoided by tagging the nodes during insertion
 */

#include "LoopAssembler.hpp"
#include "dag/Loop.hpp"
#include <queue>
#include <set>


namespace map { namespace detail {

LoopAssembler::LoopAssembler(int loop_nested_limit)
	: loop_mode(NORMAL_MODE)
	, loop_nested_limit(loop_nested_limit)
	, loop_level(-1)
	, loop_struct()
{
	// Initialize loop structures to the max. nesting limit
	loop_struct.resize(loop_nested_limit);
}

LoopMode LoopAssembler::mode() {
	return loop_mode;
}

void LoopAssembler::digestion(bool start, bool body, bool again, bool end) {
	LoopStruct &stru = loop_struct[loop_level];

	if (start) // Starts the process of digesting a loop
	{
		assert(loop_mode == NORMAL_MODE); // @ not covering nested case
		loop_level++;
		clear();
		loop_mode = LOOP_START;
	}
	else if (body) // The condition has been digested, next is the body
	{
		assert(loop_mode == LOOP_START);
		assert(not stru.cond.empty());
		loop_mode = LOOP_BODY;
	}
	else if (again) // Digested the body again to find the feedback links
	{
		assert(loop_mode == LOOP_BODY);
		assert(not stru.body.empty());
		loop_mode = LOOP_AGAIN;
	}
	else if (end) // Ends the digestion and assembles the final Loop node
	{
		assert(loop_mode == LOOP_AGAIN);
		loop_level--;
		loop_mode = NORMAL_MODE;
	}
	else {
		assert(0);
	}
}

void LoopAssembler::clear() {
	loop_struct[loop_level].prev.clear();
	loop_struct[loop_level].cond.clear();
	loop_struct[loop_level].body.clear();
	loop_struct[loop_level].again.clear();
	loop_struct[loop_level].feed_in.clear();
	loop_struct[loop_level].feed_out.clear();
	loop_struct[loop_level].loop = nullptr;
	loop_struct[loop_level].head.clear();
	loop_struct[loop_level].tail.clear();
	loop_struct[loop_level].oldpy.clear();
	loop_struct[loop_level].newpy.clear();
}

void LoopAssembler::addNode(Node *node, Node *orig) {
	LoopStruct &stru = loop_struct[loop_level];

	// Dont add node to loop if it was repeated (i.e. orig != node)
	// Unless we are AGAIN and the original node was included in 'body'
	//  - This is necessary to identify the out-loop-invariant nodes
	bool add_node = (orig == node) || (mode() == LOOP_AGAIN && is_included(orig,stru.body));
	
	if (add_node) // Stores nodes first, assembles them later
	{
		if (mode() == LOOP_START)
		{
			assert(0); //stru.cond.push_back(node);
		}
		else if (mode() == LOOP_BODY)
		{
			stru.body.push_back(orig);
		}
		else if (mode() == LOOP_AGAIN)
		{
			stru.again.push_back(orig);
		}
		else {
			assert(0);
		}
	}
}

void LoopAssembler::condition(Node *node) {
	loop_struct[loop_level].cond.push_back(node);
}

Node* LoopAssembler::assemble() {
	assert(loop_mode == LOOP_AGAIN);
	LoopStruct &stru = loop_struct[loop_level];
	int i;

	// Note: the order of the nodes inside the lists is important,
	//       do not apply optimizations (e.g. set) that break that order !!

	assert(stru.cond.size() == 2);
	Node *cond_node = stru.cond[0]; // 'cond' node that activated the Python while loop
	stru.prev.push_back(stru.cond[0]); // 'cond' is the first 'prev' and thus first 'feed_in'
	stru.feed_out.push_back(stru.cond[1]); // 'again-cond' is the first 'feed_out'

	// 'prev' of 'loop' are all those 'prev' to 'body', but outside 'body'
	for (auto node : stru.body)
		for (auto prev : node->prevList())
			if (!is_included(prev,stru.body) && !is_included(prev,stru.prev))
				stru.prev.push_back(prev);

	// The constant 'prev' are the 'prev' of 'again' outside 'body'+'again'
	std::set<Node*> const_set; // No need for order here, so std::set is ok
	for (auto node : stru.again)
		for (auto prev : node->prevList())
			if (!is_included(prev,stru.body) && !is_included(prev,stru.again))
				const_set.insert(prev);
	NodeList const_list = NodeList(const_set.begin(),const_set.end());

	// Remove from 'again' all nodes repeated in 'body'. They were used to find the invariants
	stru.again = left_join(stru.again,stru.body);

	// All 'body' nodes that only depend on the 'const_list' are in-loop-invariant nodes
	i = 0;
	while (i < stru.body.size()) {
		Node *node = stru.body[i++];
		// Do all 'prev' of this 'body' node only depend on 'const' nodes?
		auto pred = [&](Node *n){ return is_included(n,const_list); };
		bool is_invar = std::all_of(node->prevList().begin(),node->prevList().end(),pred);
		// yes? Then 'node' is a in-loop-invariant, move it out of 'body'
		if (is_invar) {
			remove_value(node,stru.body);
			stru.prev.push_back(node);
			const_list.push_back(node);
			i--;
		}
	}

	// Some 'const' / 'prev' of invariants might not be 'prev' anymore
	i = 0;
	while (i < const_list.size()) {
		Node *node = const_list[i++];
		// Does any 'next' of this 'const' node depends on 'body' ?
		auto pred = [&](Node *n){ return is_included(n,stru.body); };
		bool is_prev = std::any_of(node->nextList().begin(),node->nextList().end(),pred);
		// no? Then this 'const' is not 'prev' of the 'loop' anymore
		if (not is_prev) {
			remove_value(node,const_list);
			remove_value(node,stru.prev);
			i--;
		}
	}

	// 'feed_in' nodes are those 'prevs' not found on the constant list
	stru.feed_in = left_join(stru.prev,const_list);

	// 'feed_out' nodes are those reused between iterations
	for (auto node : stru.again)
		for (auto prev : node->prevList())
			if (is_included(prev,stru.body) && !is_included(prev,stru.feed_out))
				stru.feed_out.push_back(prev);

	// Note: 'feed_in' and 'feed_out' must maintain same size and --> order <--
	assert(stru.feed_in.size() == stru.feed_out.size());
	assert(not stru.feed_in.empty());

	// Unlinks 'again' nodes, now that we have figured out the feedbacks
	for (auto it=stru.again.rbegin(); it!=stru.again.rend(); it++) {
		Node *node = *it;
		// Nobody should link here
		assert(node->nextList().empty());
		// Inform prev nodes
		for (auto &prev : node->prevList())
			prev->removeNext(node);
		node->prev_list.clear();
		// The 'again' nodes are not deleted just yet, Python still points to them.
		// With loopUpdateVars() Python updates the loop variables to 'tail' nodes
		// which lets the garbage collector delete the 'again' nodes, sometime later
		Node::id_count--; // @ I'd be fired for this
	}

	// Unreachable 'body' nodes when going up from 'feed_out' are out-loop-invariants
	std::queue<Node*> queue;
	std::set<Node*> unr_set;
	for (auto out : stru.feed_out)
		queue.push(out);
	while (not queue.empty()) {
		Node *node = queue.front();
		queue.pop();
		for (auto prev : node->prevList())
			if (is_included(prev,stru.body))
				queue.push(prev);
		unr_set.insert(node);
	}

	// Gets the out-loop-invariants as the left_join of 'body' with the 'unreachable'
	NodeList out_invar = left_join(stru.body, NodeList(unr_set.begin(),unr_set.end()) );
	
	// Removes the out-loop-invariants from 'body' and 'again' (NB: they share the same index)
	assert(stru.body.size() == stru.again.size());
	i = 0;
	while (i < stru.body.size()) {
		if (is_included(stru.body[i],out_invar)) {
			//
			stru.oldpy.push_back(stru.again[i]);
			stru.newpy.push_back(stru.body[i]);
			//
			stru.body.erase(stru.body.begin()+i);
			stru.again.erase(stru.again.begin()+i);
		} else {
			i++;
		}
	}

	// The 'loop' node is returned to runtime for the insertion and simplification
	Node *node = Loop::Factory(stru.prev,cond_node,stru.body,stru.feed_in,stru.feed_out);
	return node;
}

void LoopAssembler::updateVars(Node *node, Node ***oldpy, Node ***newpy, int *num) {
	LoopStruct &stru = loop_struct[loop_level];

	Loop *loop_node = dynamic_cast<Loop*>(node);
	assert(loop_node != nullptr);
	stru.loop = loop_node;

	// 'head' nodes are not used at the moment, maybe in the future?
	stru.head.reserve(loop_node->headList().size());
	for (auto head : loop_node->headList())
		stru.head.push_back(head);
	// 'tail' nodes are sent back to Python to update its variables
	stru.tail.reserve(loop_node->tailList().size());
	for (auto tail : loop_node->tailList())
		stru.tail.push_back(tail);

	// These structures store what nodes the python variables should point to
	stru.oldpy.insert(stru.oldpy.end(),stru.again.begin(),stru.again.end());
	stru.newpy.insert(stru.newpy.end(),stru.tail.begin(),stru.tail.end());

	// Returns the 'oldpy' / 'newpy' nodes and their 'num'ber of nodes
	*oldpy = stru.oldpy.data();
	*newpy = stru.newpy.data();
	*num = stru.oldpy.size();

	assert(*num == stru.newpy.size());
}

} } // namespace map::detail
