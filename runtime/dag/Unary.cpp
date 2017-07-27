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

Node* Unary::Factory(Node *prev, UnaryType type) {
	assert(prev != nullptr);

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	GroupSize gs = prev->groupsize();
	
	if (type.isBitwise())
		assert(dt.isUnsigned());
	if (type.isRelational())
		dt = U8; // @ because B8 is an INT in OpenCL

	MetaData meta(ds,dt,mo,bs,gs);

	// simplification here? e.g. if prev is a constant, return a Constant Node

	return new Unary(meta,prev,type);
}

Node* Unary::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Unary(this,other_to_this);
}

// Constructors

Unary::Unary(const MetaData &meta, Node *prev, UnaryType type) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev); // [0]
	prev->addNext(this);
	
	this->type = type;

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
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

std::string Unary::shortName() const {
	return "Unary";
}

std::string Unary::longName() const {
	auto tstr = type.toString();
	auto pid = std::to_string(prev()->id);
	std::string str = "Unary {" + tstr + "," + pid + "}";
	return str;
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

void Unary::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	auto *node = this;

	auto pval = hash.find(node->prev())->second;
	hash[node] = type.apply(pval);
}

void Unary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();
	ValFix zero = ValFix( VariantType(0,datatype()) );

	auto monotonic = [](ValFix prev, UnaryType type) {
		ValFix vf = ValFix();
		vf.min = type.apply(prev.min);
		vf.max = type.apply(prev.max);
		vf.mean = type.apply(prev.mean);
		vf.std = prev.std;
		vf.active = true;
		assert(not vf.fixed);
		return vf;
	};

	auto prev = hash.find({node->prev(),coord})->second;

	// If the value is fixed, apply the operation
	if (prev.fixed) {
		hash[{node,coord}] = ValFix(node->type.apply(prev.value));
		return;
	}

	// Else, if it does not present statistics, nothing to do
	if (not prev.active) {
		hash[{node,coord}] = ValFix();
		return;
	}

	// Else (both active / perhaps one fixed), try arithmetic simplifications
	switch (type.get()) {
		case NONE_UNARY: assert(0);
		case POS: vf = prev; break;
		case NEG: {
			vf.min = type.apply(prev.max); // -max
			vf.max = type.apply(prev.min); // -min
			vf.mean = type.apply(prev.mean); // -std
			vf.std = prev.std;
			vf.active = true;
			assert(not vf.fixed);
			break;
		}
		case NOT: {
			vf.min = type.apply(prev.max); // !max
			vf.max = type.apply(prev.min); // !min
			vf.mean = type.apply(prev.mean); // !mean
			vf.std = zero.std;
			vf.active = true;
			assert(not vf.fixed);
			break;
		}
		case bNOT: break;
		case SIN: break;
		case COS: break;
		case TAN: break;
		case ASIN: break;
		case ACOS: break;
		case ATAN: break;
		case SINH: break;
		case COSH: break;
		case TANH: break;
		case ASINH: break;
		case ACOSH: break;
		case ATANH: break;
		case EXP: vf = monotonic(prev,type); break;
		case EXP2: break;
		case EXP10: break;
		case LOG: break;
		case LOG2: break;
		case LOG10: break;
		case SQRT: vf = monotonic(prev,type); break;
		case CBRT: break;
		case ABS: break;
		case CEIL: break;
		case FLOOR: break;
		case TRUNC: break;
		case ROUND: break;
	}

	//assert(vf.active);

	hash[{node,coord}] = vf;
}

} } // namespace map::detail
