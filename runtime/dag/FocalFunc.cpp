/**
 * @file	FocalFunc.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "FocalFunc.hpp"
#include "../Runtime.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

FocalFunc::Key::Key(FocalFunc *node) {
	prev = node->prev();
	smask = node->mask();
	type = node->type;
}

bool FocalFunc::Key::operator==(const Key& k) const {
	return (prev==k.prev && smask==k.smask && type==k.type);
}

std::size_t FocalFunc::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ k.smask.hash() ^ std::hash<int>()(k.type.get());
}

// Factory

Node* FocalFunc::Factory(Node *arg, const Mask &mask, ReductionType type) {
	assert(arg != nullptr);
	assert(arg->numdim() != D0);
	assert(type != NONE_REDUCTION);
	assert(arg->numdim() == mask.numdim());

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new FocalFunc(meta,arg,mask,type);
}

Node* FocalFunc::clone(NodeList new_prev_list) {
	return new FocalFunc(this,new_prev_list);
}

// Constructors

FocalFunc::FocalFunc(const MetaData &meta, Node *prev, const Mask &mask, ReductionType type)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	this->smask = mask;
	this->type = type;
	
	prev->addNext(this);
}

FocalFunc::FocalFunc(const FocalFunc *other, NodeList new_prev_list)
	: Node(other,new_prev_list)
{
	this->smask = other->smask;
	this->type = other->type;
}

// Methods

void FocalFunc::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string FocalFunc::getName() const {
	return "FocalFunc";
}

std::string FocalFunc::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += mask().signature();
	sign += type.toString();
	return sign;
}
/*
Node*& FocalFunc::prev() {
	return prev_list[0];
}
*/
Node* FocalFunc::prev() const {
	return prev_list[0];
}

Mask FocalFunc::mask() const {
	return smask;
}

BlockSize FocalFunc::halo() const {
	return smask.datasize() / 2;
}

} } // namespace map::detail
