/**
 * @file	Access.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Access.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Access::Content::Content(Access *node) {
	prev = node->prev();
	coord = node->coord();
}

bool Access::Content::operator==(const Content& k) const {
	return (prev==k.prev && all(coord==k.coord));
}

std::size_t Access::Hash::operator()(const Content& k) const {
	size_t hash = std::hash<Node*>()(k.prev);
	for (int i=0; i<k.coord.size(); i++)
		hash ^= std::hash<int>()(k.coord[i]);
	return hash;
}

// Factory

Node* Access::Factory(Node *prev, const Coord &coord) {
	assert(prev != nullptr);
	assert(prev->numdim().toInt() == coord.size());

	DataSize ds = DataSize(0); // D0
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = BlockSize(0);
	GroupSize gs = GroupSize(0);
	MetaData meta(ds,dt,mo,bs,gs);

	return new Access(meta,prev,coord);
}

Node* Access::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Access(this,other_to_this);
}

// Constructors

Access::Access(const MetaData &meta, Node *prev, const Coord &coord) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);
	
	this->cell_coord = coord;

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Access::Access(const Access *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->cell_coord = other->cell_coord;
}

// Methods

void Access::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Access::shortName() const {
	return "Access";
}

std::string Access::longName() const {
	std::string str = "Access {" + std::to_string(prev()->id) + "}";
	return str;
}

std::string Access::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += to_string(coord());
	return sign;
}

Node* Access::prev() const {
	return prev_list[0];
}

Coord Access::coord() const {
	return cell_coord;
}

} } // namespace map::detail
