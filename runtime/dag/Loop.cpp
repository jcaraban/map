/**
 * @file	Loop.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: would it be better to create Feedback nodes only at the job, during execution?
 */

#include "Loop.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Loop::Key::Key(Loop *node) {
	prev_list = node->prev_list;
	cond_list = node->prev_list;
	body_list = node->prev_list;
}

bool Loop::Key::operator==(const Key& k) const {
	if (prev_list.size() != k.prev_list.size())
		return false;
	if (cond_list.size() != k.cond_list.size())
		return false;
	if (body_list.size() != k.body_list.size())
		return false;
	bool b = true;
	for (int i=0; i<prev_list.size(); i++)
		b &= prev_list[i] == k.prev_list[i];
	for (int i=0; i<cond_list.size(); i++)
		b &= cond_list[i] == k.cond_list[i];
	for (int i=0; i<body_list.size(); i++)
		b &= body_list[i] == k.body_list[i];
	return b;
}

std::size_t Loop::Hash::operator()(const Key& k) const {
	size_t h = 0;
	for (auto &node : k.prev_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.cond_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.body_list)
		h ^= std::hash<Node*>()(node);
	return h;
}

// Factory

Node* Loop::Factory(NodeList prev_list, Node *cond_node, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list) {
	assert(not body_list.empty());
	assert(not is_included(cond_node,body_list));

	DataSize ds = {};
	NumDim nd = NONE_NUMDIM;
	DataType dt = NONE_DATATYPE;
	MemOrder mo = NONE_MEMORDER;
	BlockSize bs = {};

	// Find the least bounding attributes
	for (auto node : full_join(prev_list,body_list)) // @ cond_list too?
	{
		if (nd == NONE_NUMDIM || nd.toInt() < node->numdim().toInt()) {
			nd = node->numdim();
			ds = node->datasize();
			bs = node->blocksize();
		}
	}

	MetaData meta(ds,dt,mo,bs);
	
	return new Loop(meta,prev_list,cond_node,body_list,feed_in_list,feed_out_list);
}

// Constructors & methods

Loop::Loop(const MetaData &meta, NodeList prev_list, Node *cond_node, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list)
	: Node(meta)
{
	assert(not prev_list.empty());
	assert(not body_list.empty());
	assert(cond_node != nullptr);
	assert(feed_in_list.size() == feed_out_list.size());

	// 'body' is owned by Runtime::node_list, might change in the future?
	this->body_list = body_list;
	
	// Creates a 'head' node per 'prev' node outside 'loop'
	for (auto prev : prev_list) {
		auto head = new LoopHead(this,prev);
		this->head_list.push_back(head);

		// Creates a 'feedback-in' node per 'head' in the 'feed_in' group
		if (is_included(prev,feed_in_list)) {
			auto feed = new Feedback(this,head);
			this->feed_in_list.push_back(feed);

			// Creates a single 'cond' node hanging from one 'feed'
			if (prev == cond_node)
				this->cond_node = new LoopCond(this,feed);
		}
	}

	// 'cond' is the only 'prev' of 'loop'. This connects the dependency chain
	this->prev_list.resize(1);
	this->prev_list[0] = this->cond_node;
	this->cond_node->addNext(this);

	// Creates a 'feedback-out' node per link back into the loop, and connect it to a 'feedback-in'
	for (int i=0; i<feed_out_list.size(); i++)
		this->feed_out_list.push_back( new Feedback(this,this->feed_in_list[i],feed_out_list[i]) );

	// Creates a 'tail' per node inside the loop 'body', nodes with 'feedback' are a special case
	for (auto node : body_list) {
		Node *prev = node;
		for (auto next : node->nextList())
			if (dynamic_cast<Feedback*>(next))
				prev = next;
		this->tail_list.push_back( new LoopTail(this,prev) );
	}

	// assert ?
}

Loop::~Loop() {
	// ... continue ... nodes to unlink, delete?
	/*
	delete cond_node;
	for (auto head : head_list)
		delete head;
	for (auto feed : feed_in_list)
		delete feed;
	for (auto feed : feed_out_list)
		delete feed;
	for (auto tail : tail_list)
		delete tail;
	*/
}

void Loop::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Loop::getName() const {
	return "Loop";
}

std::string Loop::signature() const {
	std::string sign = "";
	sign += classSignature();
	for (auto prev : prev_list) {
		sign += prev->numdim().toString();
		sign += prev->datatype().toString();
	}
	return sign;
}

LoopCond* Loop::condition() const {
	return cond_node;
}

const NodeList& Loop::bodyList() const {
	return body_list;
}

const HeadList& Loop::headList() const {
	return head_list;
}

const TailList& Loop::tailList() const {
	return tail_list;
}

} } // namespace map::detail
