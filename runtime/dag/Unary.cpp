/**
 * @file	Unary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Unary.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Unary::Content::Content(Unary *node) {
	prev = node->prev();
	type = node->type;
}

bool Unary::Content::operator==(const Content& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t Unary::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* Unary::Factory(Node *arg, UnaryType type) {
	assert(arg != nullptr);

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();

	if (type.isBitwise())
		assert(dt.isUnsigned());
	if (type.isRelational())
		dt = U8; // @ because B8 is an INT in OpenCL

	MetaData meta(ds,dt,mo,bs);

	// simplification here? e.g. if arg is a constant, return a Constant Node

	return new Unary(meta,arg,type);
}

Node* Unary::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Unary(this,other_to_this);
}

// Constructors

Unary::Unary(const MetaData &meta, Node *prev, UnaryType type) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev); // [0]
	this->type = type;
	
	prev->addNext(this);
}

Unary::Unary(const Unary *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void Unary::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Unary::getName() const {
	return "Unary";
}

std::string Unary::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* Unary::prev() const {
	return prev_list[0];
}

// Compute

void Unary::computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash) {
	assert(numdim() == D0);
	Coord coord = {0,0};
	auto *node = this;

	auto pval = hash.find({node->prev(),coord})->second;
	hash[{node,coord}] = type.apply(pval);
}

void Unary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = {{},false};

	auto prev = hash.find({node->prev(),coord})->second;
	if (prev.fixed)
		vf = {node->type.apply(prev.value),true};
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
