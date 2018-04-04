/**
 * @file	Constant.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Constant.hpp"
#include "../../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Constant::Content::Content(Constant *node) {
	num_dim = node->numdim();
	cnst = node->cnst;
}

bool Constant::Content::operator==(const Content& k) const {
	return num_dim==k.num_dim && cnst.isEqual(k.cnst);
}

std::size_t Constant::Hash::operator()(const Content& k) const {
	return std::hash<int>()(k.num_dim.get()) ^ k.cnst.hash();
}

// Factory

Node* Constant::Factory(VariantType val, DataSize ds, DataType dt, MemOrder mo, BlockSize bs, GroupSize gs) {
	assert(val.datatype() == dt);
	MetaData meta(ds,dt,mo,bs,gs);
	return new Constant(meta,val);
}

Node* Constant::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Constant(this,other_to_this);
}

// Constructors

Constant::Constant(const MetaData &meta, VariantType val)
	: Node(meta)
{
	this->cnst = val;
	
	this->value = this->cnst;
	this->in_spatial_reach = Mask(); // empty
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Constant::Constant(const Constant *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->cnst = other->cnst;
}

// Methods

void Constant::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Constant::shortName() const {
	return "Constant";
}

std::string Constant::longName() const {
	std::string str = "Constant {" + value.toString() + "}";
	return str;
}

std::string Constant::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	sign += cnst.toString();
	return sign;
}

// Compute

void Constant::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	hash[this] = cnst;
}

void Constant::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = ValFix(cnst);
}

} } // namespace map::detail
