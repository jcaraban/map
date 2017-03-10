/**
 * @file	Feedback.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Feedback.hpp"
#include "LoopHead.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Feedback::Key::Key(Feedback *node) {
	prev = node->prev();
	loop = node->loop();
}

bool Feedback::Key::operator==(const Key& k) const {
	return (prev==k.prev && loop==k.loop);
}

std::size_t Feedback::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<Loop*>()(k.loop);
}

// Constructors & methods

Feedback::Feedback(Loop *loop, LoopHead *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();

	owner_loop = loop;
	in_or_out = true;
	prev_list.resize(1);
	prev_list[0] = prev;
	
	/* Feed In constructor */

	// 'next' of 'prev' inside the loop 'cond'+'body' now link to 'prev' (= head)
	for (auto next : prev->nextList()) {
		assert( is_included(next,loop->bodyList()) || next==loop->condition() );
		this->addNext(next);
		next->updatePrev(prev,this);
	}
	
	// 'prev' looses the links into 'body'
	int i = 0;
	while (i < prev->nextList().size()) {
		Node *next = prev->nextList()[i++];
		if (is_included(next,loop->bodyList()) || next==loop->condition()) {
			prev->removeNext(next);
			i--;
		}
	}

	// 'loop' owns 'feedback', so no need for it to point here
	prev->addNext(this); // 'prev' is a 'head' that points to 'feed'
}

Feedback::Feedback(Loop *loop, Feedback *feed_in, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();
	//stats = prev->datastats();

	owner_loop = loop;
	in_or_out = false;
	prev_list.resize(1);
	prev_list[0] = prev;
	
	/* Feed Out constructor */
	
	// 'prev' does not looses any 'next', it continues as it was

	// 'loop' owns 'feedback', so no need for it to point here
	prev->addNext(this); // 'prev' is a 'body' node that points to 'feed'

	// Links twin feedback nodes
	feed_in->twin = this; // feed_in --> feed_out
	this->twin = feed_in; // feed_out --> feed_in
	feed_in->prev_list.push_back(this);
	this->addNext(feed_in);
	/*
	feed_in->prev_both = full_join(feed_in->prev_list,this->prev_list);
	this->prev_both = feed_in->prev_both;
	feed_in->next_both = full_join(feed_in->next_list,this->next_list);
	this->next_both = feed_in->next_both;
	*/
}

void Feedback::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Feedback::getName() const {
	return "Feedback";
}

std::string Feedback::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

Loop* Feedback::loop() const {
	return owner_loop;
}

Node* Feedback::prev() const {
	return prev_list[0];
}

bool Feedback::feedIn() const {
	return in_or_out;
}

bool Feedback::feedOut() const {
	return ! in_or_out;
}

/*
const NodeList& Feedback::prevList() const {
	return prev_both;
}

const NodeList& Feedback::nextList() const {
	return next_both;
}
*/
} } // namespace map::detail
