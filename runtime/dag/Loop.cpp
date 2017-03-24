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
	cond_node = node->cond_node;
	prev_list = node->prev_list;
	body_list = node->body_list;
	head_list = node->head_list;
	tail_list = node->tail_list;
	feed_in_list = node->feed_in_list;
	feed_out_list = node->feed_out_list;
}

bool Loop::Key::operator==(const Key& k) const {
	if (prev_list.size() != k.prev_list.size())
		return false;
	if (body_list.size() != k.body_list.size())
		return false;
	if (head_list.size() != k.head_list.size())
		return false;
	if (tail_list.size() != k.tail_list.size())
		return false;
	if (feed_in_list.size() != k.feed_in_list.size())
		return false;
	if (feed_out_list.size() != k.feed_out_list.size())
		return false;
	bool b = cond_node == k.cond_node;
	for (int i=0; i<prev_list.size(); i++)
		b &= prev_list[i] == k.prev_list[i];
	for (int i=0; i<body_list.size(); i++)
		b &= body_list[i] == k.body_list[i];
	for (int i=0; i<head_list.size(); i++)
		b &= head_list[i] == k.head_list[i];
	for (int i=0; i<tail_list.size(); i++)
		b &= tail_list[i] == k.tail_list[i];
	for (int i=0; i<feed_in_list.size(); i++)
		b &= feed_in_list[i] == k.feed_in_list[i];
	for (int i=0; i<feed_out_list.size(); i++)
		b &= feed_out_list[i] == k.feed_out_list[i];
	return b;
}

std::size_t Loop::Hash::operator()(const Key& k) const {
	size_t h = std::hash<Node*>()(k.cond_node);
	for (auto &node : k.prev_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.body_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.head_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.tail_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.feed_in_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.feed_out_list)
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

Node* Loop::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new Loop(this,new_prev_list,new_back_list);
}

// Constructors

Loop::Loop(const MetaData &meta, NodeList prev_list, Node *cond_node, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list)
	: Node(meta)
{
	assert(not prev_list.empty());
	assert(not body_list.empty());
	assert(cond_node != nullptr);
	assert(feed_in_list.size() == feed_out_list.size());

	// The 'body' (and other) nodes are owned by Runtime::node_list
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

	// Creates a 'feedback-out' node per back-edge in the loop, and connect it to a 'feedback-in'
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

Loop::Loop(const Loop *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
{
	this->cond_node = other->cond_node;
	this->body_list = other->body_list;
	this->head_list = other->head_list;
	this->tail_list = other->tail_list;
	this->feed_in_list = other->feed_in_list;
	this->feed_out_list = other->feed_out_list;
	this->gen_num_dim = other->gen_num_dim;
	this->gen_data_size = other->gen_data_size;
	this->gen_block_size = other->gen_block_size;
	this->gen_num_block = other->gen_num_block;
}

// Methods

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
