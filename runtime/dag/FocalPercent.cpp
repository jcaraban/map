/**
 * @file	FocalPercent.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "FocalPercent.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

FocalPercent::Key::Key(FocalPercent *node) {
	prev = node->prev();
	smask = node->smask;
	type = node->type;
}

bool FocalPercent::Key::operator==(const Key& k) const {
	return (prev==k.prev && smask==k.smask && type==k.type);
}

std::size_t FocalPercent::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ k.smask.hash() ^ std::hash<int>()(k.type.get());
}

// Factory

Node* FocalPercent::Factory(Node *arg, const Mask &mask, PercentType type) {
	assert(arg != nullptr);
	assert(arg->numdim() != D0);
	assert(type != NONE_PERCENT);
	assert(arg->numdim() == mask.numdim());

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new FocalPercent(meta,arg,mask,type);
}

// Constructors & methods

FocalPercent::FocalPercent(const MetaData &meta, Node *prev, const Mask &mask, PercentType type) : Node(meta) {
	prev_list.resize(1);
	prev_list[0] = prev;
	this->smask = mask;
	this->type = type;
	
	prev->addNext(this);
}

void FocalPercent::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string FocalPercent::getName() const {
	return "FocalPercent";
}

std::string FocalPercent::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += mask().signature();
	sign += type.toString();
	return sign;
}
/*
Node*& FocalPercent::prev() {
	return prev_list[0];
}
*/
Node* FocalPercent::prev() const {
	return prev_list[0];
}

Mask FocalPercent::mask() const {
	return smask;
}

BlockSize FocalPercent::halo() const {
	return smask.datasize() / 2;
}

} } // namespace detail, map
