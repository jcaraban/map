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

// Constructors & methods

LoopHead::LoopHead(Loop *loop, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();

	prev_list.resize(1);
	prev_list[0] = prev;
	
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

	// 'loop' owns 'head', so no need for it to point to 'head'
	prev->addNext(this); // 'prev' points to 'head', not to 'loop'
}

void LoopHead::accept(Visitor *visitor) {
	//visitor->visit(this);
	assert(0);
}

std::string LoopHead::getName() const {
	return "LoopHead";
}

std::string LoopHead::signature() const {
	assert(0);
	return "";
}

Loop* LoopHead::loop() const {
	assert(0);
	//return dynamic_cast<Loop*>(prev_list[0]);
}

Node* LoopHead::prev() const {
	return prev_list[1];
}

} } // namespace map::detail
