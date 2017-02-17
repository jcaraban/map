/**
 * @file	Index.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Index.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Index::Key::Key(Index *node) {
	dim = node->dim;
}

bool Index::Key::operator==(const Key& k) const {
	return (dim==k.dim);
}

std::size_t Index::Hash::operator()(const Key& k) const {
	return std::hash<int>()(k.dim.get());
}

// Factory

Node* Index::Factory(DataSize ds, NumDim dim, MemOrder mo, BlockSize bs) {
	DataType dt = S64;
	MetaData meta(ds,dt,mo,bs);
	return new Index(meta,dim);
}

Node* Index::Factory(Node *arg, NumDim dim) {
	assert(arg != nullptr);
	DataSize ds = arg->datasize();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	
	return Factory(ds,dim,mo,bs);
}

// Constructors & methods

Index::Index(const MetaData &meta, NumDim dim) : Node(meta) {
	this->dim = dim;
}

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

} } // namespace map::detail
