/**
 * @file	LhsAccess.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LhsAccess.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LhsAccess::Content::Content(LhsAccess *node) {
	lprev = node->left();
	rprev = node->right();
	coord = node->coord();
}

bool LhsAccess::Content::operator==(const Content& k) const {
	return (lprev==k.lprev && rprev==k.rprev && all(coord==k.coord));
}

std::size_t LhsAccess::Hash::operator()(const Content& k) const {
	size_t hash = std::hash<Node*>()(k.lprev) ^ std::hash<Node*>()(k.rprev);
	for (int i=0; i<k.coord.size(); i++)
		hash ^= std::hash<int>()(k.coord[i]);
	return hash;
}

// Factory

Node* LhsAccess::Factory(Node *lhs, Node *rhs, const Coord &coord) {
	assert(lhs != nullptr && rhs != nullptr);
	assert(lhs->datatype() == rhs->datatype());
	assert(rhs->numdim() != D0);
	assert(rhs->numdim() == D0);
	assert(lhs->numdim().toInt() == coord.size());

	DataSize ds = lhs->datasize();
	DataType dt = lhs->datatype();
	MemOrder mo = lhs->memorder();
	BlockSize bs = lhs->blocksize();
	GroupSize gs = lhs->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	return new LhsAccess(meta,lhs,rhs,coord);
}

Node* LhsAccess::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new LhsAccess(this,other_to_this);
}

// Constructors

LhsAccess::LhsAccess(const MetaData &meta, Node *lprev, Node *rprev, const Coord &coord)
	: Node(meta)
{
	prev_list.reserve(2);
	this->addPrev(lprev);
	this->addPrev(rprev);
	lprev->addNext(this);
	rprev->addNext(this);

	this->cell_coord = coord;
	
	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

LhsAccess::LhsAccess(const LhsAccess *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->cell_coord = other->cell_coord;
}

// Methods

void LhsAccess::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LhsAccess::getName() const {
	return "LhsAccess";
}

std::string LhsAccess::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += left()->numdim().toString();
	sign += left()->datatype().toString();
	sign += right()->numdim().toString();
	sign += right()->datatype().toString();
	sign += to_string(coord());
	return sign;
}

Node* LhsAccess::left() const {
	return prev_list[0]; // first element
}

Node* LhsAccess::right() const {
	return prev_list[1]; // second element
}

Coord LhsAccess::coord() const {
	return cell_coord;
}

} } // namespace map::detail
