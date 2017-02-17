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
	_coord = node->coord();
}

bool LhsAccess::Key::operator==(const Key& k) const {
	return (lprev==k.lprev && rprev==k.rprev && all(_coord==k._coord));
}

std::size_t LhsAccess::Hash::operator()(const Key& k) const {
	size_t hash = std::hash<Node*>()(k.lprev) ^ std::hash<Node*>()(k.rprev);
	for (int i=0; i<k._coord.size(); i++)
		hash ^= std::hash<int>()(k._coord[i]);
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

// Constructors & methods

LhsAccess::LhsAccess(const MetaData &meta, Node *lprev, Node *rprev, const Coord &coord) : Node(meta) {
	prev_list.resize(2);
	prev_list[0] = lprev;
	prev_list[1] = rprev;
	this->_coord = coord;
	
	lprev->addNext(this);
	rprev->addNext(this);
}

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
	return _coord;
}

} } // namespace map::detail
