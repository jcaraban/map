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

	DataType data_type;
	MemOrder mem_order;
	DataShape shape;
	DataShape lshp = lhs->metadata().getDataShape();
	DataShape rshp = rhs->metadata().getDataShape();
	
	if (type.isBitwise()) {
		assert(lhs->datatype().isUnsigned());
		assert(rhs->datatype().isUnsigned());
	}

	if (lhs->numdim() == D0) {
		shape = rshp;
		mem_order = rhs->memorder();
	} else if (rhs->numdim() == D0) {
		shape = lshp;
		mem_order = lhs->memorder();
	} else {
		assert(lshp == rshp);
		assert(lhs->memorder() == rhs->memorder());
		shape = lshp;
		mem_order = lhs->memorder();
	}
	if (type.isRelational()) {
		data_type = U8; // @ because B8 is an INT in OpenCL
	} else {
		data_type = promote(lhs->datatype(),rhs->datatype());
	}
	MetaData meta(shape.data_size,data_type,mem_order,shape.block_size,shape.group_size);

	// @ identity functionality goes here ?

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
	lprev->addNext(this);
	rprev->addNext(this);
	
	this->type = type;

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
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

void Binary::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	auto *node = this;

	auto lval = hash.find(left())->second;
	auto rval = hash.find(right())->second;
	hash[node] = type.apply(lval,rval);
}

void Binary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();
	ValFix vf0 = ValFix( VariantType(0,datatype()) );
	
	auto lval = hash.find({left(),coord})->second.value;
	auto lfix = hash.find({left(),coord})->second.fixed;
	auto rval = hash.find({right(),coord})->second.value;
	auto rfix = hash.find({right(),coord})->second.fixed;

	if (lfix && rfix)
		vf = ValFix(type.apply(lval,rval));
	else if (type == MUL && lfix && lval.isZero())
		vf = vf0;
	else if (type == MUL && rfix && rval.isZero())
		vf = vf0;
	else if (type == GT && lfix && lval.isZero()) // @@
		vf = vf0;
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
