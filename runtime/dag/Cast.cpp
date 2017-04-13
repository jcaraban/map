/**
 * @file	Cast.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Cast.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Cast::Content::Content(Cast *node) {
	prev = node->prev();
	type = node->type;
}

bool Cast::Content::operator==(const Content& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t Cast::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* Cast::Factory(Node *prev, DataType new_type) {
	assert(prev != nullptr);
	assert(new_type != NONE_DATATYPE);

	DataSize ds = prev->datasize();
	DataType dt = new_type;
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Cast(meta,prev);
}

Node* Cast::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Cast(this,other_to_this);
}

// Constructors

Cast::Cast(const MetaData &meta, Node *prev)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev); // [0]
	this->type = meta.getDataType();
	
	prev->addNext(this);
}

Cast::Cast(const Cast *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void Cast::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Cast::getName() const {
	return "Cast";
}

std::string Cast::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* Cast::prev() const {
	return prev_list[0];
}

// Compute

void Cast::computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash) {
	assert(numdim() == D0);
	Coord coord = {0,0};
	auto pval = hash.find({prev(),coord})->second;
	hash[{this,coord}] = pval.convert(type);
}

void Cast::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto pval = hash.find({prev(),coord})->second.value;
	auto pfix = hash.find({prev(),coord})->second.fixed;
	if (pfix) {
		hash[{this,coord}] = {pval.convert(type),true};
	} else  {
		hash[{this,coord}] = {{},false};
	}
}

} } // namespace map::detail
