/**
 * @file	BoundedNeighbor.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "BoundedNeighbor.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

BoundedNeighbor::Content::Content(BoundedNeighbor *node) {
	prev = node->prev();
	cx = node->prev_list[1];
	cy = node->prev_list[2];
}

bool BoundedNeighbor::Content::operator==(const Content& k) const {
	return (prev==k.prev && cx==k.cx && cy==k.cy);
}

std::size_t BoundedNeighbor::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<Node*>()(k.cx) ^ std::hash<Node*>()(k.cy);
}

// Factory

Node* BoundedNeighbor::Factory(Node *prev, Node *cx, Node* cy) {
	assert(prev != nullptr and cx != nullptr and cy != nullptr);
	assert(prev->numdim() != D0);
	assert(prev->numdim() == cx->numdim());
	assert(cx->numdim() == cy->numdim());
	assert(not cx->datatype().isFloating());
	assert(not cy->datatype().isFloating());

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	GroupSize gs = prev->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	return new BoundedNeighbor(meta,prev,cx,cy);
}

Node* BoundedNeighbor::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new BoundedNeighbor(this,other_to_this);
}

// Constructors

BoundedNeighbor::BoundedNeighbor(const MetaData &meta, Node *prev, Node *cx, Node* cy)
	: Node(meta)
{
	prev_list.reserve(3);
	this->addPrev(prev);
	this->addPrev(cx);
	this->addPrev(cy);
	prev->addNext(this);
	cx->addNext(this);
	cy->addNext(this);

	Array<bool> tmask = { 1,1,1,1,1,1,1,1,1 };
	this->in_spatial_reach = Mask({3,3},tmask);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

BoundedNeighbor::BoundedNeighbor(const BoundedNeighbor *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	// ??
}

// Methods

void BoundedNeighbor::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string BoundedNeighbor::getName() const {
	return "BoundedNeighbor";
}

std::string BoundedNeighbor::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += prev_list[1]->numdim().toString();
	sign += prev_list[1]->datatype().toString();
	sign += prev_list[2]->numdim().toString();
	sign += prev_list[2]->datatype().toString();
	return sign;
}

Node* BoundedNeighbor::prev() const {
	return prev_list[0];
}

Node* BoundedNeighbor::coordx() const {
	return prev_list[1];
}

Node* BoundedNeighbor::coordy() const {
	return prev_list[2];
}

// Compute

void BoundedNeighbor::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	ValFix vf = ValFix();
	auto prev = hash.find({node->prev(),coord})->second;
	auto cx = hash.find({node->coordx(),coord})->second;
	auto cy = hash.find({node->coordy(),coord})->second;

	if (cx.fixed && cy.fixed) {
		auto xint = cx.value.convert(S32).get<S32>();
		auto yint = cy.value.convert(S32).get<S32>();
		auto neig_coord = coord + Coord{xint,yint};
		auto neig = hash.find({node->prev(),neig_coord})->second;

		if (prev.fixed && neig.fixed && prev.value == neig.value) {
			vf = ValFix(prev.value);
		}
	}
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
