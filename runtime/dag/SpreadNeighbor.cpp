/**
 * @file	SpreadNeighbor.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "SpreadNeighbor.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

SpreadNeighbor::Content::Content(SpreadNeighbor *node) {
	prev = node->prev();
	dir = node->dir();
}

bool SpreadNeighbor::Content::operator==(const Content& k) const {
	return (prev==k.prev && dir==k.dir);
}

std::size_t SpreadNeighbor::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<Node*>()(k.dir) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* SpreadNeighbor::Factory(Node *arg, Node *dir, ReductionType type) {
	assert(arg != nullptr);
	assert(arg->numdim() != D0);
	assert(arg->numdim() == dir->numdim());
	assert(dir != nullptr);
	assert(dir->numdim() != D0);
	assert(type != NONE_REDUCTION);

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new SpreadNeighbor(meta,arg,dir,type);
}

Node* SpreadNeighbor::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new SpreadNeighbor(this,other_to_this);
}

// Constructors

SpreadNeighbor::SpreadNeighbor(const MetaData &meta, Node *prev, Node *dir, ReductionType type)
	: Node(meta)
{
	prev_list.reserve(2);
	this->addPrev(prev);
	this->addPrev(dir);
	prev->addNext(this);
	dir->addNext(this);

	this->type = type;
	
	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	Array<bool> tmask = { 1,1,1,1,1,1,1,1,1 };
	this->out_spatial_reach = Mask({3,3},tmask);
}

SpreadNeighbor::SpreadNeighbor(const SpreadNeighbor *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void SpreadNeighbor::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string SpreadNeighbor::getName() const {
	return "SpreadNeighbor";
}

std::string SpreadNeighbor::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += dir()->numdim().toString();
	sign += dir()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* SpreadNeighbor::prev() const {
	return prev_list[0];
}

Node* SpreadNeighbor::dir() const {
	return prev_list[1];
}

// Compute

} } // namespace map::detail
