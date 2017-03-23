/**
 * @file	LhsAccess.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LhsAccess.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LhsAccess::Key::Key(LhsAccess *node) {
	lprev = node->left();
	rprev = node->right();
	coord = node->coord();
}

bool LhsAccess::Key::operator==(const Key& k) const {
	return (lprev==k.lprev && rprev==k.rprev && all(coord==k.coord));
}

std::size_t LhsAccess::Hash::operator()(const Key& k) const {
	size_t hash = std::hash<Node*>()(k.lprev) ^ std::hash<Node*>()(k.rprev);
	for (int i=0; i<k.coord.size(); i++)
		hash ^= std::hash<int>()(k.coord[i]);
	return hash;
}

// Factory

Node* LhsAccess::Factory(Node *lhs, Node *rhs, const Coord &coord) {
	assert(lhs != nullptr && rhs != nullptr);
	assert(lhs->datatype() == rhs->datatype());
	assert(rhs->numdim() == D0);
	assert(lhs->numdim().toInt() == coord.size());

	DataSize ds = lhs->datasize();
	DataType dt = lhs->datatype();
	MemOrder mo = lhs->memorder();
	BlockSize bs = lhs->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new LhsAccess(meta,lhs,rhs,coord);
}

Node* LhsAccess::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new LhsAccess(this,new_prev_list,new_back_list);
}

// Constructors

LhsAccess::LhsAccess(const MetaData &meta, Node *lprev, Node *rprev, const Coord &coord)
	: Node(meta)
{
	prev_list.reserve(2);
	this->addPrev(lprev);
	this->addPrev(rprev);
	this->cell_coord = coord;
	
	lprev->addNext(this);
	rprev->addNext(this);
}

LhsAccess::LhsAccess(const LhsAccess *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
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
/*
Node*& LhsAccess::left() {
	return prev_list[0]; // first element
}
*/
Node* LhsAccess::left() const {
	return prev_list[0]; // first element
}
/*
Node*& LhsAccess::right() {
	return prev_list[1]; // second element
}
*/
Node* LhsAccess::right() const {
	return prev_list[1]; // second element
}

Coord LhsAccess::coord() const {
	return cell_coord;
}

} } // namespace map::detail
