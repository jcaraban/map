/**
 * @file	Cast.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Cast.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Cast::Key::Key(Cast *node) {
	prev = node->prev();
	type = node->type;
}

bool Cast::Key::operator==(const Key& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t Cast::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* Cast::Factory(Node *prev, DataType new_type) {
	assert(prev != nullptr);
	assert(new_type != NONE_DATATYPE);

	DataSize ds = prev->datasize();
	DataType dt = new_type;
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Cast(meta,prev);
}

Node* Cast::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new Cast(this,new_prev_list,new_back_list);
}

// Constructors

Cast::Cast(const MetaData &meta, Node *prev)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev); // [0]
	this->type = meta.getDataType();
	
	prev->addNext(this);
}

Cast::Cast(const Cast *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
{
	this->type = other->type;
}

// Methods

void Cast::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Cast::getName() const {
	return "Cast";
}

std::string Cast::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}
/*
Node*& Cast::prev() {
	return prev_list[0];
}
*/
Node* Cast::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
