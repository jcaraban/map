/**
 * @file	Stats.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Stats.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Stats::Key::Key(Stats *node) {
	prev = node->prev();
}

bool Stats::Key::operator==(const Key& k) const {
	return (prev==k.prev);
}

std::size_t Stats::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Stats::Factory(Node *arg) {
	assert(arg != nullptr);

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Stats(meta,arg);
}

// Constructors & methods

Stats::Stats(const MetaData &meta, Node *prev) { //: Node(meta) {
	prev_list.resize(3);
	prev_list[0] = prev;
	prev_list[1] = ZonalReduc::Factory(prev,MAX); // Extra nodes for the
	prev_list[2] = ZonalReduc::Factory(prev,MIN); // reduction operations

	// Does not call :Node(meta) to get the id after ZonalReduc nodes
	this->id = id_count++;
	this->meta = meta;

	// Prepares statistics structure
	this->stats.active = true;
	const int num = prod(numblock());
	this->stats.maxb.resize(num);
	this->stats.meanb.resize(num);
	this->stats.minb.resize(num);
	this->stats.stdb.resize(num);

	this->prev()->addNext(this);
	this->max()->addNext(this);
	this->min()->addNext(this);
}

void Stats::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Stats::getName() const {
	return "Stats";
}

std::string Stats::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	return sign;
}
/*
Node*& Stats::prev() {
	return prev_list[0];
}
*/
Node* Stats::prev() const {
	return prev_list[0];
}
/*
Node*& Stats::max() {
	return prev_list[1]; // second element
}
*/
Node* Stats::max() const {
	return prev_list[1]; // second element
}
/*
Node*& Stats::min() {
	return prev_list[2]; // 3rd element
}
*/
Node* Stats::min() const {
	return prev_list[2]; // 3rd element
}

} } // namespace map::detail
