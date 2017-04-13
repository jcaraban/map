/**
 * @file	BoundedNbh.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "BoundedNbh.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

BoundedNbh::Content::Content(BoundedNbh *node) {
	prev = node->prev();
	cx = node->prev_list[1];
	cy = node->prev_list[2];
}

bool BoundedNbh::Content::operator==(const Content& k) const {
	return (prev==k.prev && cx==k.cx && cy==k.cy);
}

std::size_t BoundedNbh::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<Node*>()(k.cx) ^ std::hash<Node*>()(k.cy);
}

// Factory

Node* BoundedNbh::Factory(Node *prev, Node *cx, Node* cy) {
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
	MetaData meta(ds,dt,mo,bs);

	return new BoundedNbh(meta,prev,cx,cy);
}

Node* BoundedNbh::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new BoundedNbh(this,other_to_this);
}

// Constructors

BoundedNbh::BoundedNbh(const MetaData &meta, Node *prev, Node *cx, Node* cy)
	: Node(meta)
{
	prev_list.reserve(3);
	this->addPrev(prev);
	this->addPrev(cx);
	this->addPrev(cy);
	
	prev->addNext(this);
	cx->addNext(this);
	cy->addNext(this);
}

BoundedNbh::BoundedNbh(const BoundedNbh *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	// ??
}

// Methods

void BoundedNbh::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string BoundedNbh::getName() const {
	return "BoundedNbh";
}

std::string BoundedNbh::signature() const {
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

Node* BoundedNbh::prev() const {
	return prev_list[0];
}

Node* BoundedNbh::coordx() const {
	return prev_list[1];
}

Node* BoundedNbh::coordy() const {
	return prev_list[2];
}

BlockSize BoundedNbh::halo() const {
	return BlockSize{1,1};//,1,1}; // @
}

// Compute

void BoundedNbh::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	ValFix vf = {{},false};
	auto prev = hash.find({node->prev(),coord})->second;
	auto cx = hash.find({node->coordx(),coord})->second;
	auto cy = hash.find({node->coordy(),coord})->second;

	if (cx.fixed && cy.fixed) {
		auto xint = cx.value.convert(S32).get<S32>();
		auto yint = cy.value.convert(S32).get<S32>();
		auto neig_coord = coord + Coord{xint,yint};
		auto neig = hash.find({node->prev(),neig_coord})->second;

		if (prev.fixed && neig.fixed && prev.value == neig.value) {
			vf = {prev.value,true};
		}
	}
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
