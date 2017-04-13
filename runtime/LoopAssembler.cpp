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
 *       and we can use this information to avoid their Empty+Head+Merge+Switch+Tail nodes
 * TODO: is_included is "expensive", it can be avoided by tagging the nodes during insertion
 */

#include "LoopAssembler.hpp"
#include "dag/LoopCond.hpp"
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
	// Unless we are AGAIN and the original node was included in 'body'
	//  - This is necessary to identify the out-loop-invariant nodes
	bool add_node = (orig == node) || (mode() == LOOP_AGAIN && is_included(orig,stru.body));
	
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
		for (auto &prev : node->prevList())
			prev->removeNext(node);
		node->prev_list.clear();
		// The 'again' nodes are not deleted just yet, Python still points to them.
		// With loopUpdateVars() Python updates the loop variables to 'tail' nodes
		// which lets the garbage collector delete the 'again' nodes, sometime later
		Node::id_count--; // @ a more elegant way of restoring the counter ?
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
			if (is_included(prev,stru.body))
				queue.push(prev);
		reachable.insert(node);
	}

	// Gets the 'unreachable' out-loop-invariants as the left_join of 'body' with the 'reachable'
	NodeList out_invar = left_join(stru.body, NodeList(reachable.begin(),reachable.end()) );
	
	// Removes the out-loop-invariants from 'body' and 'again' (NB: they share the same index)
	assert(stru.body.size() == stru.again.size());
	i = 0;
	while (i < stru.body.size()) {
		if (is_included(stru.body[i],out_invar)) {
			// First stores the unique pointer value, for python
			stru.oldpy.push_back(stru.again[i]);
			stru.newpy.push_back(stru.body[i]);
			// Then erases them from the list
			stru.body.erase(stru.body.begin()+i);
			stru.again.erase(stru.again.begin()+i);
		} else {
			i++;
		}
	}
}

void LoopAssembler::compose() {
	LoopStruct &stru = loop_struct[loop_level];
	NodeList empty_list, empty_body, iden_list, back_list;

	auto swap_next_nodes = [&](Node *prev, Node *dest, bool neg=false) {
		// 'next' of 'prev' inside 'body' now hang from 'dest'
		int i = 0;
		while (i < prev->nextList().size()) {
			Node *next = prev->nextList()[i++];
			if ((is_included(next,stru.body) && next!=dest) ^ neg) {
				dest->addNext(next);
				next->updatePrev(prev,dest);
				prev->removeNext(next);
				i--;
			}
		}
	};

	int fix = stru.circ_out.size(); // feed-in/out
	int num_elem = stru.prev.size() + stru.body.size() - fix;

	// Re-adjusting ssa ids with these offsets
	int jmp_empty = stru.body.size() - fix; // empty nodes
	int jmp_loop = num_elem * 3 + 1; // head + merge + switch + cond nodes
	int jmp_iden = stru.prev.size() - fix; // identity nodes
	int jmp_body = stru.body.size(); // body nodes
	int jmp_tail = stru.body.size(); // tail nodes

	// The nodes created below are owned by Runtime::node_list later

	// Re-adjust ssa ids
	Node::id_count -= jmp_body; // @

	// Completes the 'prev' list with auxiliar 'empty' input nodes
	for (auto node : left_join(stru.body,stru.circ_out)) {
		// This should be an Empty node, not a Const // @
		auto *empty = new Constant(node->metadata(),VariantType(0,node->datatype()));
		stru.prev.push_back(empty);
		stru.other.push_back(empty);

		empty_list.push_back(empty);
		empty_body.push_back(node);
	}

	// Finds the feed-back 'body' node per 'prev'
	for (int i=0; i<stru.prev.size(); i++) {
		Node *prev = stru.prev[i];
		Node *back = nullptr;

		if (is_included(prev,stru.circ_in))
		{	// This is a feed-in 'prev', so a 'body' node exists in 'feed-out'
			int i = value_position(prev,stru.circ_in);
			back = stru.circ_out[i];
		}
		else if (is_included(prev,empty_list))
		{	// This is a 'tmp' node in body, an 'empty' node was created
			int i = value_position(prev,empty_list);
			back = empty_body[i];
		}
		else
		{	// is a const-input-node
			auto *iden = Identity::Factory(prev);
			stru.body.push_back(iden);
			iden_list.push_back(iden);
			stru.other.push_back(iden);
			swap_next_nodes(prev,iden);
			back = iden;

			back->id += jmp_loop;
			Node::id_count--; // @
		}
		back_list.push_back(back);
	}

	// Re-adjust 'body' ids
	for (auto node : stru.body)
		if (not is_included(node,iden_list))
			node->id += jmp_empty + jmp_loop + jmp_iden;

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

		auto *merge = dynamic_cast<Merge*>( Merge::Factory(head,back) );
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

	Node::id_count += jmp_body; // @

	// Creates a 'tail' node hanging from the 'false' side of 'switch'
	for (int i=0; i<stru.switc.size(); i++) {
		Node *node = back_list[i];
		Switch *swit = stru.switc[i];

		if (not is_included(node,iden_list)) {
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
