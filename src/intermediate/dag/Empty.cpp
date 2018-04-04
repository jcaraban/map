/**
 * @file	Empty.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Empty.hpp"
#include "../../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Empty::Content::Content(Empty *node) {
	num_dim = node->numdim();
}

bool Empty::Content::operator==(const Content& k) const {
	return num_dim==k.num_dim;
}

std::size_t Empty::Hash::operator()(const Content& k) const {
	return std::hash<int>()(k.num_dim.get());
}

// Factory

Node* Empty::Factory(DataSize ds, DataType dt, MemOrder mo, BlockSize bs, GroupSize gs) {
	MetaData meta(ds,dt,mo,bs,gs);
	return new Empty(meta);
}

Node* Empty::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Empty(this,other_to_this);
}

// Constructors

Empty::Empty(const MetaData &meta)
	: Node(meta)
{
	this->value = VariantType(); // none
	this->in_spatial_reach = Mask(); // empty
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Empty::Empty(const Empty *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Empty::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Empty::shortName() const {
	return "Empty";
}

std::string Empty::longName() const {
	return "Empty";
}

std::string Empty::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

// Compute

void Empty::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	hash[this] = VariantType();
}

void Empty::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = ValFix(VariantType(0,datatype())); // @@
}

} } // namespace map::detail
