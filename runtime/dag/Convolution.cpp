/**
 * @file	Convolution.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Convolution.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Convolution::Key::Key(Convolution *node) {
	prev = node->prev();
	smask = node->mask();
}

bool Convolution::Key::operator==(const Key& k) const {
	return (prev==k.prev && smask==k.smask);
}

std::size_t Convolution::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ k.smask.hash();
}

// Factory

Node* Convolution::Factory(Node *arg, const Mask &mask) {
	assert(arg != nullptr);
	//assert(mask != nullptr); // some other checking for mask?
	assert(arg->numdim() != D0);
	assert(arg->numdim() == mask.numdim());

	DataSize ds = arg->datasize();
	DataType dt = promote(arg->datatype(),mask.datatype());
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Convolution(meta,arg,mask);
}

Node* Convolution::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new Convolution(this,other_to_this);
}

// Constructors

Convolution::Convolution(const MetaData &meta, Node *prev, const Mask &mask)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	this->smask = mask;
	
	prev->addNext(this);
}

Convolution::Convolution(const Convolution *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	this->smask = other->smask;
}

// Methods

void Convolution::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Convolution::getName() const {
	return "Convolution";
}

std::string Convolution::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += mask().signature();
	return sign;
}
/*
Node*& Convolution::prev() {
	return prev_list[0];
}
*/
Node* Convolution::prev() const {
	return prev_list[0];
}

Mask Convolution::mask() const {
	return smask;
}

BlockSize Convolution::halo() const {
	return smask.datasize() / 2;
}

} } // namespace map::detail
