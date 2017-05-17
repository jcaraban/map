/**
 * @file	Convolution.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: inputReach() should be filled according to mask
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

Node* Convolution::Factory(Node *prev, const Mask &mask) {
	assert(prev != nullptr);
	//assert(mask != nullptr); // some other checking for mask?
	assert(prev->numdim() != D0);
	assert(prev->numdim() == mask.numdim());

	DataSize ds = prev->datasize();
	DataType dt = promote(prev->datatype(),mask.datatype());
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	GroupSize gs = prev->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	return new Convolution(meta,prev,mask);
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
	prev->addNext(this);
	
	this->smask = mask;

	auto mds = mask.datasize();
	auto marr = Array<bool>(prod(mds));
	for (int i=0; i<marr.size(); i++)
		marr[i] = mask.array[i] ? true : false;

	this->in_spatial_reach = Mask(mds,marr);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
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

// Compute

void Convolution::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	auto bt = BinaryType(MUL); // @
	auto rt = ReductionType(SUM); // @
	auto reach = inputReach(coord);
	auto bs = blocksize();

	auto prev = hash.find({node->prev(),coord})->second;
	if (not prev.fixed) {
		hash[{node,coord}] = ValFix();
		return;
	}

	for (auto offset : reach.blockSpace(bs)) {
		Coord nbc = coord + offset;
		auto neig = hash.find({node->prev(),nbc})->second;
		if (not neig.fixed || neig.value != prev.value) {
			hash[{node,coord}] = ValFix();
			return;
		}
	}

	// At this point all 'prev blocks' contain the same fixed value
	auto val = prev.value;
	auto mask = node->mask();
	auto acu = rt.neutral(node->datatype());

	for (auto offset : reach.cellSpace())
	{	
		auto aux = bt.apply(val,mask[offset]);
		acu = rt.apply(acu,aux);
	}

	hash[{node,coord}] = ValFix(acu);
}

} } // namespace map::detail
