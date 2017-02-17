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

// Constructors & methods

LoopTail::LoopTail(Loop *loop, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();

	prev_list.resize(2);
	prev_list[0] = loop;
	prev_list[1] = prev;
	
	// 'prev' should not point to anybody 'next' at this point
	//assert( prev->nextList().empty() );

	loop->addNext(this); // @ take care with this complex linking...
	prev->addNext(this); // 'prev' is a 'feedback' that points to 'tail'
}

void LoopTail::accept(Visitor *visitor) {
	//visitor->visit(this);
	assert(0);
}

std::string LoopTail::getName() const {
	return "LoopTail";
}

std::string LoopTail::signature() const {
	assert(0);
	return "";
}

Loop* LoopTail::loop() const {
	return dynamic_cast<Loop*>(prev_list[0]);
}

Node* LoopTail::prev() const {
	return prev_list[1];
}

} } // namespace map::detail
