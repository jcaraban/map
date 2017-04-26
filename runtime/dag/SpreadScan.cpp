/**
 * @file	SpreadScan.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "SpreadScan.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

SpreadScan::Content::Content(SpreadScan *node) {
	prev = node->prev();
	dir = node->dir();
	type = node->type;
}

bool SpreadScan::Content::operator==(const Content& k) const {
	return (prev==k.prev && dir==k.dir && type==k.type);
}

std::size_t SpreadScan::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<Node*>()(k.dir) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* SpreadScan::Factory(Node *prev, Node *dir, ReductionType type) {
	assert(prev != nullptr);
	assert(dir != nullptr);
	assert(prev->numdim() != D0);
	assert(all(prev->datasize() == dir->datasize()));
	assert(dir->datatype() == U8); // Directions in 2D are U8

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new SpreadScan(meta,prev,dir,type);
}

Node* SpreadScan::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new SpreadScan(this,other_to_this);
}

// Constructors

SpreadScan::SpreadScan(const MetaData &meta, Node *prev, Node *dir, ReductionType type)
	: Node(meta)
{
	prev_list.reserve(5);
	this->addPrev(prev);
	this->addPrev(dir);
	this->type = type;
	// Temporal node for the Spread raster (e.g. water)
	// @ TODO: spread = Copy::Factory(prev);
	Node *spread = new Temporal(prev->metadata());
//	prev_list[2] = addNode(spread);
	Node *buffer = new Temporal(prev->metadata());
//	prev_list[3] = addNode(buffer);
	// Temporal node to store the stability (if cells changed)
	// @ TODO: stable = Constant::Factory(type.neutral(datatype()),prev->datasize(),U8);
	MetaData aux_meta = prev->metadata();
	aux_meta.data_type = U16;
	Node *stable = new Temporal(aux_meta);
//	prev_list[4] = addNode(stable);
	
	prev->addNext(this);
	dir->addNext(this);
	spread->addNext(this);
	buffer->addNext(this);
	stable->addNext(this);
}

SpreadScan::SpreadScan(const SpreadScan *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void SpreadScan::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string SpreadScan::getName() const {
	return "SpreadScan";
}

std::string SpreadScan::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += dir()->numdim().toString();
	sign += dir()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* SpreadScan::prev() const {
	return prev_list[0];
}

Node* SpreadScan::dir() const {
	return prev_list[1]; // second element
}

Node* SpreadScan::spread() const {
	return prev_list[2]; // third element
}

Node* SpreadScan::buffer() const {
	return prev_list[3]; // 4th element
}

Node* SpreadScan::stable() const {
	return prev_list[4]; // 5th element
}

// Compute

} } // namespace map::detail
