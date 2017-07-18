/**
 * @file	BlockSummary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "BlockSummary.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

BlockSummary::Content::Content(BlockSummary *node) {
	prev = node->prev();
	type = node->type;
}

bool BlockSummary::Content::operator==(const Content& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t BlockSummary::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* BlockSummary::Factory(Node *prev, ReductionType type) {
	assert(prev != nullptr);
	assert(prev->numdim() != D0);

	DataSize ds = idiv(prev->datasize(),prev->blocksize());
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->numdim().unitVec();
	GroupSize gs = prev->numdim().unitVec();
	MetaData meta(ds,dt,mo,bs,gs);

	return new BlockSummary(meta,prev,type);
}

Node* BlockSummary::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new BlockSummary(this,other_to_this);
}

// Constructors

BlockSummary::BlockSummary(const MetaData &meta, Node *prev, ReductionType type) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->type = type;
	//this->value = type.neutral(datatype());
	
	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true); // @ shall be something else ?
}

BlockSummary::BlockSummary(const BlockSummary *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void BlockSummary::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string BlockSummary::getName() const {
	return "BlockSummary";
}

std::string BlockSummary::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += to_string(prev()->datasize()); // @
	sign += to_string(prev()->blocksize()); // @
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* BlockSummary::prev() const {
	return prev_list[0];
}

// Compute

VariantType BlockSummary::initialValue() const {
	return type.neutral(datatype());
}

void BlockSummary::updateValue(VariantType value) {
	//node->type.atomic(node->value,blk->value); // @
	this->value = type.apply(this->value,value);
}

void BlockSummary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();

	auto prev = hash.find({node->prev(),coord})->second;
	if (prev.fixed) {
		switch (node->type.get()) {
			case SUM:  vf = ValFix( BinaryType(MUL).apply( prev.value , prod(blocksize()) ) ); break;
			case PROD: vf = ValFix( BinaryType(POW).apply( prev.value , prod(blocksize()) ) ); break;
			case rAND: vf = ValFix(prev.value); break;
			case rOR:  vf = ValFix(prev.value); break;
			case MAX:  vf = ValFix(prev.value); break;
			case MIN:  vf = ValFix(prev.value); break;
			default: assert(0);
		}
	}
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
