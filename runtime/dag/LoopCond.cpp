/**
 * @file	LoopCond.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LoopCond.hpp"
#include "Loop.hpp"
#include "../Runtime.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LoopCond::Key::Key(LoopCond *node) {
	prev = node->prev();
	loop = node->loop();
}

bool LoopCond::Key::operator==(const Key& k) const {
	return (prev==k.prev && loop==k.loop);
}

std::size_t LoopCond::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<Loop*>()(k.loop);
}

// Constructors & methods

LoopCond::LoopCond(Loop *loop, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();
	meta.data_type = U8; // @ because OpenCL

	owner_loop = loop;
	prev_list.resize(1);
	prev_list[0] = prev;
	
	prev->addNext(this); // 'prev' is a previously existing node
	// 'loop' will point to 'cond' in order to close the dependency chain
}

void LoopCond::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LoopCond::getName() const {
	return "LoopCond";
}

std::string LoopCond::signature() const {
	assert(0);
	return "";
}

Loop* LoopCond::loop() const {
	return owner_loop;
}

Node* LoopCond::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
