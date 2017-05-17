/**
 * @file	Barrier.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Barrier.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Barrier::Content::Content(Barrier *node) {
	prev = node->prev();
}

bool Barrier::Content::operator==(const Content& k) const {
	return (prev==k.prev);
}

std::size_t Barrier::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Barrier::Factory(Node *prev) {
	assert(prev != nullptr);

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	GroupSize gs = prev->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	return new Barrier(meta,prev);
}

Node* Barrier::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Barrier(this,other_to_this);
}

// Constructors

Barrier::Barrier(const MetaData &meta, Node *prev) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Barrier::Barrier(const Barrier *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Barrier::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Barrier::getName() const {
	return "Barrier";
}

std::string Barrier::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	return sign;
}

Node* Barrier::prev() const {
	return prev_list[0];
}

// Compute

void Barrier::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	hash[{node,coord}] = hash.find({node->prev(),coord})->second;
}

} } // namespace map::detail
