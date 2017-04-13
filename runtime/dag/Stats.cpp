/**
 * @file	Stats.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Stats.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Stats::Content::Content(Stats *node) {
	prev = node->prev();
}

bool Stats::Content::operator==(const Content& k) const {
	return (prev==k.prev);
}

std::size_t Stats::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Stats::Factory(Node *prev) {
	assert(prev != nullptr);

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Stats(meta,prev);
}

Node* Stats::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Stats(this,other_to_this);
}

// Constructors

Stats::Stats(const MetaData &meta, Node *prev) { //: Node(meta) {
	prev_list.reserve(3);
	this->addPrev(prev);
	this->addPrev( ZonalReduc::Factory(prev,MAX) ); // Extra nodes for the
	this->addPrev( ZonalReduc::Factory(prev,MIN) ); // reduction operations
	// @@ memory leak, Zonal must be owned by Runtime

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

Stats::Stats(const Stats *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

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

Node* Stats::prev() const {
	return prev_list[0];
}

Node* Stats::max() const {
	return prev_list[1]; // second element
}

Node* Stats::min() const {
	return prev_list[2]; // 3rd element
}

// Compute

void Stats::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	auto prev = hash.find({node->prev(),coord})->second;
	hash[{node,coord}] = {prev.value,prev.fixed};
}

} } // namespace map::detail
