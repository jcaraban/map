/**
 * @file	LoopHead.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LoopHead.hpp"
#include "Loop.hpp"
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
	return std::hash<Node*>()(k.prev) ^ std::hash<Loop*>()(k.loop);
}

Node* LoopHead::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new LoopHead(this,other_to_this);
}

// Constructors

LoopHead::LoopHead(Loop *loop, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();
	
	owner_loop = loop; // 'head' knows who its 'loop' is
	prev_list.reserve(1);
	this->addPrev(prev);
	
	// 'next' of 'prev' inside 'cond'+'body' now link to 'head'
	for (auto next : prev->nextList()) {
		if (is_included(next,loop->bodyList()) || next==loop->condition()) {
			this->addNext(next);
			next->updatePrev(prev,this);
		}
	}
	
	// 'prev' looses the links into 'cond'+'body'
	int i = 0;
	while (i < prev->nextList().size()) {
		Node *next = prev->nextList()[i++];
		if (is_included(next,loop->bodyList()) || next==loop->condition()) {
			prev->removeNext(next);
			i--;
		}
	}

	prev->addNext(this); // finally 'prev' points to 'head'
}

LoopHead::LoopHead(const LoopHead *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	this->owner_loop = nullptr; // filled later by 'loop', because it does not live yet
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

Loop* LoopHead::loop() const {
	return owner_loop;
}

Node* LoopHead::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
