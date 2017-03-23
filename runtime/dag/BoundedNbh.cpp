/**
 * @file	BoundedNbh.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "BoundedNbh.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

BoundedNbh::Key::Key(BoundedNbh *node) {
	prev = node->prev();
	cx = node->prev_list[1];
	cy = node->prev_list[2];
}

bool BoundedNbh::Key::operator==(const Key& k) const {
	return (prev==k.prev && cx==k.cx && cy==k.cy);
}

std::size_t BoundedNbh::Hash::operator()(const Key& k) const {
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

Node* BoundedNbh::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new BoundedNbh(this,new_prev_list,new_back_list);
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

BoundedNbh::BoundedNbh(const BoundedNbh *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
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
/*
Node*& BoundedNbh::prev() {
	return prev_list[0];
}
*/
Node* BoundedNbh::prev() const {
	return prev_list[0];
}

//Node* BoundedNbh::coord() const {
//	return prev_list[1];
//}

BlockSize BoundedNbh::halo() const {
	return BlockSize{1,1};//,1,1}; // @@
}

} } // namespace map::detail
