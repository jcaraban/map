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

// Feedback

Node* Feedback::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new Feedback(this,other_to_this);
}

// Constructors

Feedback::Feedback(Loop *loop, LoopHead *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();

	owner_loop = loop; // 'feed' knows who its 'loop' is
	in_or_out = true;
	prev_list.resize(1);
	prev_list[0] = prev;
	
	/* Feed In constructor */

	// 'next' of 'prev' (which must be inside 'cond'+'body') now link to 'feed'
	for (auto next : prev->nextList()) {
		assert( is_included(next,loop->bodyList()) || next==loop->condition() );
		this->addNext(next);
		next->updatePrev(prev,this);
	}

	// 'prev' looses all its 'next' links
	for (auto next : prev->nextList())
		prev->removeNext(next);

	prev->addNext(this); // 'prev' is a 'head' that ONLY points to 'feed'
}

Feedback::Feedback(Loop *loop, Feedback *feed_in, Node *prev)
	: Node()
{
	id = prev->id;
	meta = prev->metadata();
	//stats = prev->datastats();
	
	owner_loop = loop; // 'feed' knows who its 'loop' is
	in_or_out = false;
	prev_list.resize(1);
	prev_list[0] = prev;
	
	/* Feed Out constructor */
	
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

	prev->addNext(this); // 'prev' is a 'body' node that points to 'feed'

	// Links twin feedback nodes
	feed_in->twin = this; // feed_in --> feed_out
	this->twin = feed_in; // feed_out --> feed_in
	feed_in->addForw(this);
	this->addBack(feed_in);
}

Feedback::Feedback(const Feedback *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	this->owner_loop = nullptr; // nullptr because 'loop' does not live yet
	this->in_or_out = other->in_or_out;
	this->twin = other->twin;
}

// Methods

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
Pattern Feedback::pattern() const {
	return in_or_out ? HEAD : TAIL;
}

} } // namespace map::detail
