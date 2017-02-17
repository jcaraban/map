/**
 * @file	Constant.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Constant.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Constant::Key::Key(Constant *node) {
	num_dim = node->numdim();
	cnst = node->cnst;
}

bool Constant::Key::operator==(const Key& k) const {
	return num_dim==k.num_dim && cnst.isEqual(k.cnst);
}

std::size_t Constant::Hash::operator()(const Key& k) const {
	return std::hash<int>()(k.num_dim.get()) ^ k.cnst.hash();
}

// Factory

Node* Constant::Factory(VariantType var, DataSize ds, DataType dt, MemOrder mo, BlockSize bs) {
	MetaData meta(ds,dt,mo,bs);
	return new Constant(meta,var);
}

// Constructors & methods

Constant::Constant(const MetaData &meta, VariantType val) : Node(meta) {
	this->cnst = val;
	assert(val.datatype() == meta.getDataType());
}

void Constant::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Constant::getName() const {
	return "Constant";
}

std::string Constant::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	sign += cnst.toString();
	return sign;
}

} } // namespace map::detail
