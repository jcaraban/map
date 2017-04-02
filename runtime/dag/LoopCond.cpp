/**
 * @file	LoopCond.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: would it be better to create Feedback nodes only at the job, during execution?
 */

#include "LoopCond.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LoopCond::Key::Key(LoopCond *node) {
	prev = node->prev();
	//prev_list = node->prev_list;
	//body_list = node->body_list;
	head_list = node->headList();
	tail_list = node->tailList();
	//merge_list = node->merge_list;
	//switch_list = node->switch_list;
}

bool LoopCond::Key::operator==(const Key& k) const {
	//if (prev_list.size() != k.prev_list.size())
	//	return false;
	//if (body_list.size() != k.body_list.size())
	//	return false;
	if (head_list.size() != k.head_list.size())
		return false;
	if (tail_list.size() != k.tail_list.size())
		return false;
	//if (merge_list.size() != k.merge_list.size())
	//	return false;
	//if (switch_list.size() != k.switch_list.size())
	//	return false;
	bool b = prev == k.prev;
	//for (int i=0; i<prev_list.size(); i++)
	//	b &= prev_list[i] == k.prev_list[i];
	//for (int i=0; i<body_list.size(); i++)
	//	b &= body_list[i] == k.body_list[i];
	for (int i=0; i<head_list.size(); i++)
		b &= head_list[i] == k.head_list[i];
	for (int i=0; i<tail_list.size(); i++)
		b &= tail_list[i] == k.tail_list[i];
	//for (int i=0; i<merge_list.size(); i++)
	//	b &= merge_list[i] == k.merge_list[i];
	//for (int i=0; i<merge_list.size(); i++)
	//	b &= switch_list[i] == k.switch_list[i];
	return b;
}

std::size_t LoopCond::Hash::operator()(const Key& k) const {
	size_t h = std::hash<Node*>()(k.prev);
	//for (auto &node : k.prev_list)
	//	h ^= std::hash<Node*>()(node);
	//for (auto &node : k.body_list)
	//	h ^= std::hash<Node*>()(node);
	for (auto &node : k.head_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.tail_list)
		h ^= std::hash<Node*>()(node);
	//for (auto &node : k.merge_list)
	//	h ^= std::hash<Node*>()(node);
	//for (auto &node : k.switch_list)
	//	h ^= std::hash<Node*>()(node);
	return h;
}

// Factory

//Node* LoopCond::Factory(NodeList prev_list, Node *cond_node, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list) {
Node* LoopCond::Factory(Node *prev) {
	//assert(not prev_list.empty());
	//assert(not head_list.empty());
	assert(prev != nullptr);
	//assert(not body_list.empty());
	//assert(not is_included(cond_node,body_list));
	//assert(feed_in_list.size() == feed_out_list.size());

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();

	MetaData meta(ds,dt,mo,bs);
	
	//return new LoopCond(meta,prev_list,cond_node,body_list,feed_in_list,feed_out_list);
	return new LoopCond(meta,prev);
}

Node* LoopCond::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new LoopCond(this,other_to_this);
}

// Constructors

//LoopCond::LoopCond(const MetaData &meta, NodeList prev_list, Node *cond_node, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list)
LoopCond::LoopCond(const MetaData &meta, Node *prev)
	: Node(meta)
{
	this->prev_list.reserve(1);
	this->addPrev(prev);

	prev->addNext(this);
}

LoopCond::LoopCond(const LoopCond *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	/*
	this->body_list.reserve(other->body_list.size());
	for (auto other_body : other->body_list)
		this->body_list.push_back( other_to_this.find(other_body)->second );
	*/
	this->head_list.reserve(other->head_list.size());
	for (auto other_head : other->head_list) {
		Node *this_head = other_to_this.find(other_head)->second;
		this->head_list.push_back( dynamic_cast<LoopHead*>(this_head) );
		this->head_list.back()->owner_loop = this;  // the 'head' clone is completed now
	}
	
	this->tail_list.reserve(other->tail_list.size()); /*
	for (auto other_tail : other->tail_list)
		this->tail_list.push_back( other_to_this.find(other_tail)->second ); */
}

// Methods

void LoopCond::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LoopCond::getName() const {
	return "LoopCond";
}

std::string LoopCond::signature() const {
	std::string sign = "";
	sign += classSignature();
	for (auto prev : prev_list) {
		sign += prev->numdim().toString();
		sign += prev->datatype().toString();
	}
	return sign;
}

//const NodeList& LoopCond::bodyList() const {
//	return body_list;
//}

const HeadList& LoopCond::headList() const {
	return head_list;
}

const TailList& LoopCond::tailList() const {
	return tail_list;
}

//const MergeList& LoopCond::mergeList() const {
//	return merge_list;
//}

//const SwitchList& LoopCond::switchList() const {
//	return switch_list;
//}

Node* LoopCond::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
