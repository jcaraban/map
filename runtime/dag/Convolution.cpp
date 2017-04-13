/**
 * @file	Convolution.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Convolution.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Convolution::Content::Content(Convolution *node) {
	prev = node->prev();
	smask = node->mask();
}

bool Convolution::Content::operator==(const Content& k) const {
	return (prev==k.prev && smask==k.smask);
}

std::size_t Convolution::Hash::operator()(const Content& k) const {
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

Node* Convolution::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
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

Convolution::Convolution(const Convolution *other, const std::unordered_map<Node*,Node*> &other_to_this)
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

Node* Convolution::prev() const {
	return prev_list[0];
}

Mask Convolution::mask() const {
	return smask;
}

BlockSize Convolution::halo() const {
	return smask.datasize() / 2;
}

// Compute

void Convolution::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	auto prev = hash[{node->prev(),coord}];
	auto bt = BinaryType(MUL);
	auto rt = ReductionType(SUM);
	auto acu = rt.neutral(node->datatype());

	if (not prev.fixed) {
		hash[{node,coord}] = {{},false};
		return;
	}
	for (int y=-halo()[1]; y<=halo()[1]; y++) {
		for (int x=-halo()[0]; x<=halo()[0]; x++)
		{
			auto neig = hash.find({node->prev(),coord+Coord{x,y}})->second;
			if (not neig.fixed || prev.value != neig.value) {
				hash[{node,coord}] = {{},false};
				return;
			}
			int proj = (y + halo()[1]) * (2*halo()[0]+1) + (x + halo()[0]);
			auto aux = bt.apply(neig.value,node->mask()[proj]);
			acu = rt.apply(acu,aux);
		}
	}
	hash[{node,coord}] = {acu,true};
}

} } // namespace map::detail
