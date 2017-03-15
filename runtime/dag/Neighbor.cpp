/**
 * @file	Neighbor.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Neighbor.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Neighbor::Key::Key(Neighbor *node) {
	prev = node->prev();
	scoord = node->coord();
}

bool Neighbor::Key::operator==(const Key& k) const {
	return (prev==k.prev && all(scoord==k.scoord));
}

std::size_t Neighbor::Hash::operator()(const Key& k) const {
	size_t hash = std::hash<Node*>()(k.prev);
	for (int i=0; i<k.scoord.size(); i++)
		hash ^= std::hash<int>()(k.scoord[i]);
	return hash;
}

// Factory

Node* Neighbor::Factory(Node *prev, const Coord &coord) {
	assert(prev != nullptr);
	assert(prev->numdim() != D0);
	assert(prev->numdim().toInt() == coord.size());

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Neighbor(meta,prev,coord);
}

Node* Neighbor::clone(NodeList new_prev_list) {
	return new Neighbor(this,new_prev_list);
}

// Constructors

Neighbor::Neighbor(const MetaData &meta, Node *prev, const Coord &coord)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	this->scoord = coord;
	
	prev->addNext(this);
}

Neighbor::Neighbor(const Neighbor *other, NodeList new_prev_list)
	: Node(other,new_prev_list)
{
	this->scoord = other->scoord;
}

// Methods

void Neighbor::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Neighbor::getName() const {
	return "Neighbor";
}

std::string Neighbor::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += to_string(coord());
	return sign;
}
/*
Node*& Neighbor::prev() {
	return prev_list[0];
}
*/
Node* Neighbor::prev() const {
	return prev_list[0];
}

Coord Neighbor::coord() const {
	return scoord;
}

BlockSize Neighbor::halo() const {
	return abs(scoord);
}

} } // namespace map::detail
