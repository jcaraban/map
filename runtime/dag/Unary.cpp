/**
 * @file	Unary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Unary.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Unary::Key::Key(Unary *node) {
	prev = node->prev();
	type = node->type;
}

bool Unary::Key::operator==(const Key& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t Unary::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* Unary::Factory(Node *arg, UnaryType type) {
	assert(arg != nullptr);

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();

	if (type.isBitwise())
		assert(dt.isUnsigned());
	if (type.isRelational())
		dt = U8; // @ because B8 is an INT in OpenCL

	MetaData meta(ds,dt,mo,bs);

	// simplification here? e.g. if arg is a constant, return a Constant Node

	return new Unary(meta,arg,type);
}

// Constructors & methods

Unary::Unary(const MetaData &meta, Node *prev, UnaryType type) : Node(meta) {
	prev_list.resize(1);
	prev_list[0] = prev;
	this->type = type;
	
	prev->addNext(this);
}

void Unary::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Unary::getName() const {
	return "Unary";
}

std::string Unary::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}
/*
Node*& Unary::prev() {
	return prev_list[0];
}
*/
Node* Unary::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
