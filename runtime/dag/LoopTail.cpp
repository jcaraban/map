/**
 * @file	LoopTail.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LoopTail.hpp"
#include "Loop.hpp"
#include "../Runtime.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LoopTail::Key::Key(LoopTail *node) {
	prev = node->prev();
	loop = node->loop();
}

bool LoopTail::Key::operator==(const Key& k) const {
	return (prev==k.prev && loop==k.loop);
}

std::size_t LoopTail::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<Loop*>()(k.loop);
}

// Factory

Node* LoopTail::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new LoopTail(this,new_prev_list,new_back_list);
}

// Constructors

LoopTail::LoopTail(Loop *loop, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();

	owner_loop = loop;
	prev_list.resize(2);
	prev_list[0] = prev;
	prev_list[1] = loop; // @ first 'loop' or 'prev' ?
	
	// 'prev' should not point to anybody 'next' at this point
	//assert( prev->nextList().empty() );

	loop->addNext(this); // @ take care with this complex linking...
	prev->addNext(this); // 'prev' is a 'feedback' that points to 'tail'
}

LoopTail::LoopTail(const LoopTail *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
{
	this->owner_loop = other->owner_loop;
}

// Methods

void LoopTail::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LoopTail::getName() const {
	return "LoopTail";
}

std::string LoopTail::signature() const {
	assert(0);
	return "";
}

Loop* LoopTail::loop() const {
	return owner_loop;
}

Node* LoopTail::prev() const {
	return prev_list[1];
}

} } // namespace map::detail
