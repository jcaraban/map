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

// Constructors & methods

LoopCond::LoopCond(Loop *loop, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();
	meta.data_type = U8; // @ because OpenCL

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
	assert(0);
	//return dynamic_cast<Loop*>(prev_list[0]);
}

Node* LoopCond::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
