/**
 * @file	FocalFunc.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "FocalFunc.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

FocalFunc::Content::Content(FocalFunc *node) {
	prev = node->prev();
	smask = node->mask();
	type = node->type;
}

bool FocalFunc::Content::operator==(const Content& k) const {
	return (prev==k.prev && smask==k.smask && type==k.type);
}

std::size_t FocalFunc::Hash::operator()(const Content& k) const {
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

Node* FocalFunc::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new FocalFunc(this,other_to_this);
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

FocalFunc::FocalFunc(const FocalFunc *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
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

Node* FocalFunc::prev() const {
	return prev_list[0];
}

Mask FocalFunc::mask() const {
	return smask;
}

BlockSize FocalFunc::halo() const {
	return smask.datasize() / 2;
}

// Compute

void FocalFunc::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	auto prev = hash[{node->prev(),coord}];
	auto bt = BinaryType(MUL);
	auto rt = ReductionType(node->type);
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
