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

Binary::Content::Content(Binary *node) {
	lprev = node->left();
	rprev = node->right();
	type = node->type;
}

bool Binary::Content::operator==(const Content& k) const {
	return (lprev==k.lprev && rprev==k.rprev && type==k.type);
}

std::size_t Binary::Hash::operator()(const Content& k) const {
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

Node* Binary::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Binary(this,other_to_this);
}

// Constructors

Binary::Binary(const MetaData &meta, Node *lprev, Node *rprev, BinaryType type)
	: Node(meta)
{
	prev_list.reserve(2);
	this->addPrev(lprev); // pos [0]
	this->addPrev(rprev); // pos [1]
	this->type = type;
	
	lprev->addNext(this);
	rprev->addNext(this);
}

Binary::Binary(const Binary *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

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

Node* Binary::left() const {
	return prev_list[0]; // first element
}

Node* Binary::right() const {
	return prev_list[1]; // second element
}

// Compute

void Binary::computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash) {
	assert(numdim() == D0);
	Coord coord = {0,0};
	auto *node = this;

	auto lval = hash.find({left(),coord})->second;
	auto rval = hash.find({right(),coord})->second;
	hash[{node,coord}] = type.apply(lval,rval);
}

void Binary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = {{},false};
	ValFix vf0 = {VariantType(0,datatype()),true};
	
	auto lval = hash.find({left(),coord})->second.value;
	auto lfix = hash.find({left(),coord})->second.fixed;
	auto rval = hash.find({right(),coord})->second.value;
	auto rfix = hash.find({right(),coord})->second.fixed;

	if (lfix && rfix)
		vf = {type.apply(lval,rval),true};
	else if (type == MUL && lfix && lval.isZero())
		vf = vf0;
	else if (type == MUL && rfix && rval.isZero())
		vf = vf0;
	else if (type == GT && lfix && lval.isZero()) // @@
		vf = vf0;
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
