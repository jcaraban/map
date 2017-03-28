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

Node* LoopTail::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new LoopTail(this,other_to_this);
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
	
	// the 'next' of 'prev' outside 'body' are moved down
	int i = 0; // this are the out-loop-invariants
	while (i < prev->nextList().size()) {
		Node *next = prev->nextList()[i++];
		if (not is_included(next,loop->bodyList())) {
			this->addNext(next);
			next->updatePrev(prev,this);
			prev->removeNext(next);
			i--;
		}
	}

	loop->addNext(this); // 'loop' is reached through its 'tail' nodes
	prev->addNext(this); // 'prev' is a 'feedback' that points to 'tail'
}

LoopTail::LoopTail(const LoopTail *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	Node *this_loop = other_to_this.find(other->owner_loop)->second;
	this->owner_loop = dynamic_cast<Loop*>(this_loop);
	
	/// Pushes itself into 'loop', because 'tail' did not live when 'loop' was created
	this->owner_loop->tail_list.push_back(this);
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
