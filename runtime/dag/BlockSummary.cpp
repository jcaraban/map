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

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	MetaData meta(ds,dt,mo,bs);

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
	
	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
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
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* BlockSummary::prev() const {
	return prev_list[0];
}

// Compute

void BlockSummary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();

	auto prev = hash.find({node->prev(),coord})->second;
	if (prev.fixed) {
		if (node->type == MAX || node->type == MIN) {
			vf = ValFix(prev.value);
		}
		// @ SUM could be implemented as a multiplication by the block_size
		// @ PROD could be implemented as a exponentiation by the block_size
	}
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
