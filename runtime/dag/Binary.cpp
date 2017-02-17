/**
 * @file	Binary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Binary.hpp"
#include "Constant.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Binary::Key::Key(Binary *node) {
	lprev = node->left();
	rprev = node->right();
	type = node->type;
}

bool Binary::Key::operator==(const Key& k) const {
	return (lprev==k.lprev && rprev==k.rprev && type==k.type);
}

std::size_t Binary::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.lprev) ^ std::hash<Node*>()(k.rprev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* Binary::Factory(Node *lhs, Node *rhs, BinaryType type) {
	assert(lhs != nullptr);
	assert(rhs != nullptr);

	NumDim dim, ldim = lhs->numdim(), rdim = rhs->numdim();
	DataSize ds, lds = lhs->datasize(), rds = rhs->datasize();
	DataType dt, ldt = lhs->datatype(), rdt = rhs->datatype();
	MemOrder mo, lmo = lhs->memorder(), rmo = rhs->memorder();
	BlockSize bs, lbs = lhs->blocksize(), rbs = rhs->blocksize();

	if (type.isBitwise())
		assert(ldt.isUnsigned() && rdt.isUnsigned());

	if (ldim == D0) {
		dim=rdim, ds=rds, mo=rmo, bs=rbs;
	} else if (rdim == D0) {
		dim=ldim, ds=lds, mo=lmo, bs=lbs;
	} else {
		assert( ldim==rdim && all(lds==rds) && lmo==rmo && all(lbs==rbs) );
		dim=ldim, ds=lds, mo=lmo, bs=lbs;
	}
	if (type.isRelational()) {
		dt = U8; // @ because B8 is an INT in OpenCL
	} else {
		dt = DataType( promote(ldt,rdt) );
	}
	MetaData meta(ds,dt,mo,bs);

	// @ identity functionality goes here?

	return new Binary(meta,lhs,rhs,type);
}

// Constructors & methods

Binary::Binary(const MetaData &meta, Node *lprev, Node *rprev, BinaryType type) : Node(meta) {
	prev_list.resize(2);
	prev_list[0] = lprev;
	prev_list[1] = rprev;
	this->type = type;
	
	lprev->addNext(this);
	rprev->addNext(this);
}

void Binary::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Binary::getName() const {
	return "Binary";
}

std::string Binary::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += left()->numdim().toString();
	sign += left()->datatype().toString();
	sign += right()->numdim().toString();
	sign += right()->datatype().toString();
	sign += type.toString();
	return sign;
}
/*
Node*& Binary::left() {
	return prev_list[0]; // first element
}
*/
Node* Binary::left() const {
	return prev_list[0]; // first element
}
/*
Node*& Binary::right() {
	return prev_list[1]; // second element
}
*/
Node* Binary::right() const {
	return prev_list[1]; // second element
}

} } // namespace map::detail
