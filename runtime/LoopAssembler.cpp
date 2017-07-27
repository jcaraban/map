/**
 * @file    LoopAssembler.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * NOTE: the whole approach is flawed, this is a quick & dirty way of assembling loops.
 *       Instead of deleting the 'again' nodes, playing with the 'ssa_id_count',
 *       injecting the Head, Merge, Switch, Tail, Loop nodes in the middle, etc...
 *       the nodes should be created in sequential order, respecting the SSA and dataflow
 *       - For that a Python front-end needs to parse the While first, (as dag or bytecode)
 *
 * TODO: only the '{body.ref - again.ref} > 0' nodes are alive in Python,
 *       	( also 'node.ref > node.next_list.size' must be alive ? )
 *       and we can use this information to avoid their Empty+Head+Merge+Switch+Tail nodes
 * TODO: is_included is "expensive", it can be avoided by tagging the nodes during insertion
 *
 * TODO: identities keep storing/loading even when they represent constant nodes like Read
 *       - Add 'file' to all Nodes and make use of StreamDir to tag them as Read-Only
 * TODO: @@ seems to me, online-simplifying while assembling hide critical dependencies
 */

#include "LoopAssembler.hpp"
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
	loop_struct[loop_level].circ_in.clear();
	loop_struct[loop_level].circ_out.clear();
	loop_struct[loop_level].loop = nullptr;
	loop_struct[loop_level].head.clear();
	loop_struct[loop_level].tail.clear();
	loop_struct[loop_level].oldpy.clear();
	loop_struct[loop_level].newpy.clear();
}

void LoopAssembler::addNode(Node *node, Node *orig) {
	LoopStruct &stru = loop_struct[loop_level];

	// Dont add node to loop if it was repeated (i.e. orig != node)
	// Unless we are AGAIN and the original node was in 'body' but not in 'again'
	//  - This is necessary to identify the out-loop-invariant nodes
	//
	// TODO: urban_dyn.py loop not working as intended when certain nodes (e.g. zero) are repeated

	bool add_node = (orig == node) || (mode() == LOOP_AGAIN && 
					is_included(orig,stru.body) && !is_included(orig,stru.again));

	if (add_node) // Stores nodes first, assembles them later
	{
		if (mode() == LOOP_START)
		{
			assert(0); // nothing to add
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
			assert(0); // nothing to add
		}
	}
}

void LoopAssembler::condition(Node *node) {
	loop_struct[loop_level].cond.push_back(node);
}

void LoopAssembler::assemble() {
	extract();
	compose();
}

void LoopAssembler::extract() {
	assert(loop_mode == LOOP_AGAIN);
	LoopStruct &stru = loop_struct[loop_level];
	int i;

	// Note: the order of the nodes inside the lists is important,
	//       do not apply optimizations (e.g. set) that break that order !!

	assert(stru.cond.size() == 2);
	Node *cond_node = stru.cond[0]; // 'cond' node that activated the Python while loop
	stru.prev.push_back(stru.cond[0]); // 'cond' is the first 'prev' and thus first 'circ_in'
	stru.circ_out.push_back(stru.cond[1]); // 'again-cond' is the first 'circ_out'

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

	// Nodes in 'body' && 'again' only depending on 'const_list' are 'in-loop-invariants'
	i = 0;
	while (i < stru.body.size()) {
		Node *node = stru.body[i++];
		// Do all 'prev' of this 'body' node only depend on 'const' nodes?
		auto pred = [&](Node *n){ return is_included(n,const_list); };
		bool is_invar = std::all_of(node->prevList().begin(),node->prevList().end(),pred);
		// yes? Then 'node' is a in-loop-invariant, move it out of the loop
		if (is_invar && is_included(node,stru.again)) {
			remove_value(node,stru.body);
			remove_value(node,stru.again);
			stru.invar_in.push_back(node);
			stru.prev.push_back(node);
			const_list.push_back(node);
			i--;
		}
	}

	// @@ This might be unnecessary, or even break some loops
	//if (is_included(stru.cond.back(),stru.again))
	//	remove_value(stru.cond.back(),stru.again);
	//assert(inner_join(stru.body,stru.again).empty());

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

	// 'circ_in' nodes are those 'prevs' not found on the constant list
	stru.circ_in = left_join(stru.prev,const_list);

	// 'circ_out' nodes are those reused between iterations
	for (auto node : stru.again)
		for (auto prev : node->prevList())
			if (is_included(prev,stru.body) && !is_included(prev,stru.circ_out))
				stru.circ_out.push_back(prev);

	// Note: 'circ_in' and 'circ_out' must maintain same size and --> order <--
	assert(stru.circ_in.size() == stru.circ_out.size());
	assert(not stru.circ_in.empty());

	// Unlinks 'again' nodes, now that we have figured out the feedbacks
	for (auto it=stru.again.rbegin(); it!=stru.again.rend(); it++) {
		Node *node = *it;
		// Nobody should link here
		assert(node->nextList().empty());
		// Inform prev nodes
		for (auto prev : node->prevList())
			prev->removeNext(node);
		node->prev_list.clear();
		// The 'again' nodes are not deleted just yet, Python still points to them.
		// With loopUpdateVars() Python updates the loop variables to 'tail' nodes
		// which lets the garbage collector delete the 'again' nodes, sometime later
	}

	// Unreachable 'body' nodes when going up from 'circ_out' are out-loop-invariants
	std::queue<Node*> queue;
	std::set<Node*> reachable;
	for (auto out : stru.circ_out)
		queue.push(out);
	while (not queue.empty()) {
		Node *node = queue.front();
		queue.pop();
		for (auto prev : node->prevList())
			if (reachable.find(prev) == reachable.end())
				if (is_included(prev,stru.body))
					queue.push(prev);
		reachable.insert(node);
	}

	// Gets the 'unreachable' out-loop-invariants as the left_join of 'body' with the 'reachable'
	stru.invar_out = left_join(stru.body, NodeList(reachable.begin(),reachable.end()) );
	
	// Removes the out-loop-invariants from 'body' and 'again' (NB: they share the same index)
	i = 0;
	while (i < stru.body.size()) {
		if (is_included(stru.body[i],stru.invar_out)) {
			// First stores the unique pointer value, for python
			stru.oldpy.push_back(stru.again[i]);
			stru.newpy.push_back(stru.body[i]);
			// Then erases the nodes from the lists
			stru.body.erase(stru.body.begin()+i);
			stru.again.erase(stru.again.begin()+i);
		} else {
			i++;
		}
	}

	// Note: 'circ_in' and 'circ_out' must maintain same size and --> order <--
	assert(stru.body.size() == stru.again.size());
	assert(not stru.body.empty());
}

void LoopAssembler::compose() {
	LoopStruct &stru = loop_struct[loop_level];
	NodeList empty_body, back_list;

	auto swap_next_nodes = [&](Node *prev, Node *dest, bool neg=false) {
		// 'next' of 'prev' inside 'body' now hang from 'dest'
		int i = 0;
		while (i < prev->nextList().size()) {
			Node *next = prev->nextList()[i++];
			bool inside = is_included(next,stru.body) ||
						  is_included(next,stru.iden);
			if ((inside && next!=dest) ^ neg) {
				dest->addNext(next);
				next->updatePrev(prev,dest);
				prev->removeNext(next);
				i--;
			}
		}
	};

	// The nodes created below are owned by Runtime::node_list and given to it later

	// Completes the 'prev' list with auxiliar 'empty' input nodes
	for (auto node : left_join(stru.body,stru.circ_out)) {
		auto *empty = new Empty(node->metadata());
		stru.prev.push_back(empty);
		stru.empty.push_back(empty);
		empty_body.push_back(node);
	}

	// Finds the feed-back 'body' node per 'prev'
	for (int i=0; i<stru.prev.size(); i++) {
		Node *prev = stru.prev[i];
		Node *back = nullptr;

		if (is_included(prev,stru.circ_in))
		{	// This is a circ-in 'prev', so a 'body' node exists in 'circ-out'
			int j = value_position(prev,stru.circ_in);
			back = stru.circ_out[j];
		}
		else if (is_included(prev,stru.empty))
		{	// This is a 'tmp' node in body, an 'empty' node was created
			int j = value_position(prev,stru.empty);
			back = empty_body[j];
		}
		else
		{	// is a const-input-node
			auto *iden = Identity::Factory(prev);
			stru.iden.push_back(iden);
			swap_next_nodes(prev,iden);
			back = iden;
		}
		back_list.push_back(back);
	}

	// Creates a 'head' node per 'prev' node outside 'loop'
	for (int i=0; i<stru.prev.size(); i++) {
		Node *prev = stru.prev[i];

		auto *head = dynamic_cast<LoopHead*>( LoopHead::Factory(prev) );
		stru.head.push_back(head);
	}

	// Creates a 'merge' node from the 'head' and its respective 'body'
	for (int i=0; i<stru.head.size(); i++) {
		Node *head = stru.head[i];
		Node *back = back_list[i];

		auto *merge = dynamic_cast<Merge*>( Merge::Factory(head,back,MergeLoopFlag()) );
		stru.merge.push_back(merge);
	}

	// Makes this 'LoopCond' node depend on the 'merge' of the 'condition'
	for (int i=0; i<stru.merge.size(); i++) {
		Node *prev = stru.prev[i];
		Node *merge = stru.merge[i];

		if (prev == stru.cond[0]) {
			stru.loop = dynamic_cast<LoopCond*>( LoopCond::Factory(merge) );
		}
	}

	// Creates a 'switch' per 'merge', with this 'loop' as condition
	for (int i=0; i<stru.merge.size(); i++) {
		Node *prev = stru.prev[i];
		Node *merge = stru.merge[i];

		auto *swit = dynamic_cast<Switch*>( Switch::Factory(stru.loop,merge) );
		stru.switc.push_back(swit);
	}

	// Re-link 'body' nodes to the 'switch' according to their 'prev'
	for (int i=0; i<stru.prev.size(); i++) {
		Node *prev = stru.prev[i];
		Switch *swit = stru.switc[i];
		swap_next_nodes(prev,swit);
		// Moves all 'next' nodes to the 'true_side' of 'switch'
		for (auto next : swit->nextList())
			swit->addTrue(next);
	}

	// Creates a 'tail' node hanging from the 'false' side of 'switch'
	for (int i=0; i<stru.switc.size(); i++) {
		Node *node = back_list[i];
		Switch *swit = stru.switc[i];

		if (is_included(node,stru.iden))
			continue; // Identities don't need Tail

		auto tail = dynamic_cast<LoopTail*>( LoopTail::Factory(swit) );
		stru.tail.push_back(tail);
		swap_next_nodes(node,tail,true);
		// Links the twin 'head' and 'tail' nodes
		LoopHead *head = stru.head[i];
		head->twin_tail = tail;
		tail->twin_head = head;
		// Adds the 'tail' node to the 'false_side' of 'switch'
		swit->addFalse(tail);
	}
	
	// Link 'head' and 'tail' nodes with their owner 'loop'
	stru.loop->head_list = stru.head;
	stru.loop->tail_list = stru.tail;
	for (auto head : stru.head)
		head->owner_loop = stru.loop;
	for (auto tail : stru.tail)
		tail->owner_loop = stru.loop;

	// assert ?
}

void LoopAssembler::updateVars(Node *node, Node ***oldpy, Node ***newpy, int *num) {
	LoopStruct &stru = loop_struct[loop_level];
	assert(stru.loop == node);

	// 'oldpy' --> 'newpy' maps what nodes the python variables should now point to
	stru.oldpy.insert(stru.oldpy.end(),stru.again.begin(),stru.again.end());
	// Follows the 'body' nodes until their tail through
	for (auto node : stru.body) {
		Node *merge = node->backList().front();
		assert(dynamic_cast<Merge*>(merge));
		Node *swit = merge->nextList().back();
		assert(dynamic_cast<Switch*>(swit));
		Node *tail = swit->nextList().back();
		assert(dynamic_cast<LoopTail*>(tail));
		stru.newpy.push_back(tail);
	}

	// Returns the 'oldpy' / 'newpy' nodes and their 'num'ber of nodes
	*oldpy = stru.oldpy.data();
	*newpy = stru.newpy.data();
	*num = stru.oldpy.size();

	assert(*num == stru.newpy.size());
}

} } // namespace map::detail
