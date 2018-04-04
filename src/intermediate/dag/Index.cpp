/**
 * @file	Index.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Index.hpp"
#include "../../visitor/Visitor.hpp"
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

Node* Index::Factory(DataSize ds, NumDim dim, MemOrder mo, BlockSize bs, GroupSize gs) {
	assert(dim != D0 && dim != NONE_NUMDIM);

	DataType dt = S64;
	MetaData meta(ds,dt,mo,bs,gs);
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
	
	this->in_spatial_reach = Mask(); // empty
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
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

std::string Index::shortName() const {
	return "Index";
}

std::string Index::longName() const {
	std::string str = "Index {" + dim.toString() + "}";
	return str;
}

std::string Index::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += dim.toString();
	return sign;
}

// Compute

void Index::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	hash[this] = VariantType(0,datatype());
}

void Index::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	ValFix vf;
	int idx;

	if (dim == D1)
		idx = 0;
	else if (dim == D2)
		idx = 1;
	else if (dim == D3)
		idx = 2;

	vf.min = VariantType(0,datatype());
	vf.max = VariantType(datasize()[idx],datatype());
	vf.mean = (vf.min + vf.max) / 2;
	vf.std = (vf.max - vf.min) / 4;
	vf.active = true;

	hash[{this,coord}] = vf;
}

} } // namespace map::detail
