/**
 * @file	RadialScan.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "RadialScan.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

RadialScan::Key::Key(RadialScan *node) {
	prev = node->prev();
	type = node->type;
	start = node->start;
}

bool RadialScan::Key::operator==(const Key& k) const {
	return (prev==k.prev && type==k.type && all(start==k.start));
}

std::size_t RadialScan::Hash::operator()(const Key& k) const {
	size_t hash = std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
	for (int i=0; i<k.start.size(); i++)
		hash ^= std::hash<int>()(k.start[i]);
	return hash;
}

// Factory

Node* RadialScan::Factory(Node *arg, ReductionType type, Coord start) {
	assert(arg != nullptr);
	assert(arg->numdim() != D0);

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new RadialScan(meta,arg,type,start);
}

Node* RadialScan::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new RadialScan(this,new_prev_list,new_back_list);
}

// Constructors

RadialScan::RadialScan(const MetaData &meta, Node *prev, ReductionType type, Coord start)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	this->type = type;
	this->start = start;
	
	prev->addNext(this);
}

RadialScan::RadialScan(const RadialScan *other, NodeList new_prev_list, NodeList new_back_list)
	: Node(other,new_prev_list,new_back_list)
{
	this->type = other->type;
	this->start = other->start;
}

// Methods

void RadialScan::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string RadialScan::getName() const {
	return "RadialScan";
}

std::string RadialScan::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	sign += to_string(start);
	return sign;
}
/*
Node*& RadialScan::prev() {
	return prev_list[0];
}
*/
Node* RadialScan::prev() const {
	return prev_list[0];
}

} } // namespace map::detail
