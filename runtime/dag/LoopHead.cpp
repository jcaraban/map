/**
 * @file	LoopHead.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LoopHead.hpp"
#include "LoopCond.hpp"
#include "../Runtime.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LoopHead::Key::Key(LoopHead *node) {
	prev = node->prev();
	loop = node->loop();
}

bool LoopHead::Key::operator==(const Key& k) const {
	return (prev==k.prev && loop==k.loop);
}

std::size_t LoopHead::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<LoopCond*>()(k.loop);
}

// LoopHead

//Node* LoopHead::Factory(LoopCond *loop, Node *prev) {
Node* LoopHead::Factory(Node *prev) {
	MetaData meta = prev->metadata();
	return new LoopHead(meta,prev);
}

Node* LoopHead::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new LoopHead(this,other_to_this);
}

// Constructors

//LoopHead::LoopHead(const MetaData &meta, LoopCond *loop, Node *prev)
LoopHead::LoopHead(const MetaData &meta, Node *prev)
	: Node(meta)
{
	owner_loop = nullptr; // 'head' knows who its 'loop' is
	twin_tail = nullptr; // 'head' might have a twin 'tail'
	
	prev_list.reserve(1);
	this->addPrev(prev);

	prev->addNext(this);
}

LoopHead::LoopHead(const LoopHead *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	this->owner_loop = nullptr; // filled later by 'loop', because it does not live yet
}

LoopHead::~LoopHead() {
	// Notifies its 'loop' about the deletion
	remove_value(this,owner_loop->head_list);
}


// Methods

void LoopHead::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LoopHead::getName() const {
	return "LoopHead";
}

std::string LoopHead::signature() const {
	assert(0);
	return "";
}

LoopCond* LoopHead::loop() const {
	return owner_loop;
}

Node* LoopHead::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
