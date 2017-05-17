/**
 * @file	LoopTail.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LoopTail.hpp"
#include "LoopCond.hpp"
#include "../Runtime.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LoopTail::Content::Content(LoopTail *node) {
	prev = node->prev();
	loop = node->loop();
}

bool LoopTail::Content::operator==(const Content& k) const {
	return (prev==k.prev && loop==k.loop);
}

std::size_t LoopTail::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<LoopCond*>()(k.loop);
}

// Factory

Node* LoopTail::Factory(Node *prev) {
	MetaData meta = prev->metadata();
	return new LoopTail(meta,prev);
}

Node* LoopTail::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new LoopTail(this,other_to_this);
}

// Constructors

LoopTail::LoopTail(const MetaData &meta, Node *prev)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this); // 'prev' is a 'switch' that points to 'tail'

	owner_loop = nullptr; // 'tail' knows who its 'loop' is
	twin_head = nullptr; // 'tail' might have a twin 'head'

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

LoopTail::LoopTail(const LoopTail *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	Node *this_loop = other_to_this.find(other->owner_loop)->second;
	this->owner_loop = dynamic_cast<LoopCond*>(this_loop);
	
	// Pushes itself into 'loop', because 'tail' did not live when 'loop' was created
	this->owner_loop->tail_list.push_back(this);

	// Notifies its twin 'head', if any exists
	if (other->twin_head != nullptr) {
		Node *this_head = other_to_this.find(other->twin_head)->second;
		this->twin_head = dynamic_cast<LoopHead*>(this_head);
		this->twin_head->twin_tail = this;
	}
}

LoopTail::~LoopTail() {
	// Notifies its 'loop' about the deletion
	remove_value(this,owner_loop->tail_list);
}

// Methods

void LoopTail::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LoopTail::getName() const {
	return "LoopTail";
}

std::string LoopTail::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	return sign;
}

LoopCond* LoopTail::loop() const {
	return owner_loop;
}

Node* LoopTail::prev() const {
	return prev_list[0];
}

// Compute 

void LoopTail::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = hash.find({prev(),coord})->second;
}

} } // namespace map::detail
