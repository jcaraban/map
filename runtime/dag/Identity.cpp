/**
 * @file	Identity.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Identity.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Identity::Key::Key(Identity *node) {
	prev = node->prev();
}

bool Identity::Key::operator==(const Key& k) const {
	return (prev==k.prev);
}

std::size_t Identity::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Identity::Factory(Node *prev) {
	assert(prev != nullptr);
	return new Identity(prev->metadata(),prev);
}

Node* Identity::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new Identity(this,other_to_this);
}

// Constructors

Identity::Identity(const MetaData &meta, Node *prev)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	
	prev->addNext(this);
}

Identity::Identity(const Identity *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Identity::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Identity::getName() const {
	return "Identity";
}

std::string Identity::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	return sign;
}
/*
Node*& Identity::prev() {
	return prev_list[0];
}
*/
Node* Identity::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
