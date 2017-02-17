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

Node* Cast::Factory(Node *arg, DataType new_type) {
	assert(arg != nullptr);
	assert(new_type != NONE_DATATYPE);

	DataSize ds = arg->datasize();
	DataType dt = new_type;
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Cast(meta,arg);
}

// Constructors & methods

Cast::Cast(const MetaData &meta, Node *prev) : Node(meta) {
	prev_list.resize(1);
	prev_list[0] = prev;
	this->type = meta.getDataType();
	
	prev->addNext(this);
}

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
