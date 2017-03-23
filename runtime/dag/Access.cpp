/**
 * @file	Access.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Access.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Access::Key::Key(Access *node) {
	prev = node->prev();
	coord = node->coord();
}

bool Access::Key::operator==(const Key& k) const {
	return (prev==k.prev && all(coord==k.coord));
}

std::size_t Access::Hash::operator()(const Key& k) const {
	size_t hash = std::hash<Node*>()(k.prev);
	for (int i=0; i<k.coord.size(); i++)
		hash ^= std::hash<int>()(k.coord[i]);
	return hash;
}

// Factory

Node* Access::Factory(Node *arg, const Coord &coord) {
	assert(arg != nullptr);
	assert(arg->numdim().toInt() == coord.size());

	DataSize ds = {}; // D0
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = {};
	MetaData meta(ds,dt,mo,bs);

	return new Access(meta,arg,coord);
}

Node* Access::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new Access(this,new_prev_list,new_back_list);
}

// Constructors

Access::Access(const MetaData &meta, Node *prev, const Coord &coord) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev);
	this->cell_coord = coord;
	
	prev->addNext(this);
}

Access::Access(const Access *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
{
	this->cell_coord = other->cell_coord;
}

// Methods

void Access::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Access::getName() const {
	return "Access";
}

std::string Access::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += to_string(coord());
	return sign;
}
/*
Node*& Access::prev() {
	return prev_list[0];
}
*/
Node* Access::prev() const {
	return prev_list[0];
}

Coord Access::coord() const {
	return cell_coord;
}

} } // namespace map::detail
