/**
 * @file	Switch.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Switch.hpp"
#include "LoopHead.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Switch::Key::Key(Switch *node) {
	cond = node->cond();
	prev = node->prev();
}

bool Switch::Key::operator==(const Key& k) const {
	return (cond==k.cond && prev==k.prev);
}

std::size_t Switch::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.cond) ^ std::hash<Node*>()(k.prev);
}

// Switch

Node* Switch::Factory(Node *cond, Node *prev) {
	assert(cond != nullptr);
	assert(prev != nullptr);

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();

	MetaData meta(ds,dt,mo,bs);

	return new Switch(meta,cond,prev);
}

Node* Switch::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new Switch(this,other_to_this);
}

// Constructors

Switch::Switch(const MetaData &meta, Node *cond, Node *prev)
	: Node(meta)
{
	prev_list.reserve(2);
	this->addPrev(cond);
	this->addPrev(prev);
	
	cond->addNext(this);
	prev->addNext(this);
}

Switch::Switch(const Switch *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	//this-next_true =  ? ;
	//this-next_false =  ? ;
}

// Methods

void Switch::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Switch::getName() const {
	return "Switch";
}

std::string Switch::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

Node* Switch::cond() const {
	return prev_list[0];
}

Node* Switch::prev() const {
	return prev_list[1];
}

const NodeList& Switch::nextList(bool true_false) const {
	return true_false ? next_true : next_false;
}

Pattern Switch::pattern() const {
	return SWITCH;
}

} } // namespace map::detail
