/**
 * @file	GroupSummary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "GroupSummary.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

GroupSummary::Content::Content(GroupSummary *node) {
	prev = node->prev();
	type = node->type;
}

bool GroupSummary::Content::operator==(const Content& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t GroupSummary::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* GroupSummary::Factory(Node *prev, ReductionType type) {
	assert(prev != nullptr);
	assert(prev->numdim() != D0);

	DataSize ds = prev->datasize() / prev->groupsize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	GroupSize bs = prev->blocksize() / prev->groupsize();
	GroupSize gs = prev->numdim().unitVec();
	MetaData meta(ds,dt,mo,bs,gs);

	return new GroupSummary(meta,prev,type);
}

Node* GroupSummary::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new GroupSummary(this,other_to_this);
}

// Constructors

GroupSummary::GroupSummary(const MetaData &meta, Node *prev, ReductionType type) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->type = type;
	//this->value = type.neutral(datatype());
	
	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

GroupSummary::GroupSummary(const GroupSummary *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void GroupSummary::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string GroupSummary::getName() const {
	return "GroupSummary";
}

std::string GroupSummary::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* GroupSummary::prev() const {
	return prev_list[0];
}

// Compute

void GroupSummary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();

	auto prev = hash.find({node->prev(),coord})->second;
	if (prev.fixed) {
		switch (node->type.get()) {
			case SUM:  vf = ValFix( prev.value * prod(blocksize()) ); break;
			case PROD: vf = ValFix( pow(prev.value,prod(blocksize())) ); break;
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
