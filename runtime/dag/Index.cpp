/**
 * @file	Index.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Index.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Index::Content::Content(Index *node) {
	dim = node->dim;
}

bool Index::Content::operator==(const Content& k) const {
	return (dim==k.dim);
}

std::size_t Index::Hash::operator()(const Content& k) const {
	return std::hash<int>()(k.dim.get());
}

// Factory

Node* Index::Factory(DataSize ds, NumDim dim, MemOrder mo, BlockSize bs) {
	assert(dim != D0 && dim != NONE_NUMDIM);

	DataType dt = S64;
	MetaData meta(ds,dt,mo,bs);
	return new Index(meta,dim);	
}

Node* Index::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Index(this,other_to_this);
}

// Constructors

Index::Index(const MetaData &meta, NumDim dim)
	: Node(meta)
{
	this->dim = dim;
}

Index::Index(const Index *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->dim = other->dim;
}

// Methods

void Index::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Index::getName() const {
	return "Index";
}

std::string Index::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += dim.toString();
	return sign;
}

// Compute

void Index::computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash) {
	assert(numdim() == D0);
}

void Index::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = {{},false}; // never fixed
}

} } // namespace map::detail
