/**
 * @file	Neighbor.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: inputReach() should be filled according to 'scoord'
 */

#include "Neighbor.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Neighbor::Content::Content(Neighbor *node) {
	prev = node->prev();
	scoord = node->coord();
}

bool Neighbor::Content::operator==(const Content& k) const {
	return (prev==k.prev && all(scoord==k.scoord));
}

std::size_t Neighbor::Hash::operator()(const Content& k) const {
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
	GroupSize gs = prev->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	return new Neighbor(meta,prev,coord);
}

Node* Neighbor::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Neighbor(this,other_to_this);
}

// Constructors

Neighbor::Neighbor(const MetaData &meta, Node *prev, const Coord &coord)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);
	
	this->scoord = coord;

	Array<bool> tmask = { 1,1,1,1,1,1,1,1,1 }; // @
	this->in_spatial_reach = Mask({3,3},tmask);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Neighbor::Neighbor(const Neighbor *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
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

Node* Neighbor::prev() const {
	return prev_list[0];
}

Coord Neighbor::coord() const {
	return scoord;
}

// Compute

void Neighbor::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	auto prev = hash.find({node->prev(),coord})->second;
	auto neig = hash.find({node->prev(),coord+node->coord()})->second;
	ValFix vf = ValFix();

	if (prev.fixed && neig.fixed && prev.value == neig.value)
		vf = ValFix(prev.value);
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
