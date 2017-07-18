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

size_t Binary::Hash::operator()(const Content& k) const {
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
	VariantType min, max, mean, std;

	ValFix zero = ValFix( VariantType(0,datatype()) );
	ValFix one = ValFix( VariantType(1,datatype()) );
	ValFix otoz; // vales range from "one to zero"
	otoz.min = zero.min;
	otoz.max = one.max;
	otoz.mean = zero.mean;
	otoz.std = zero.std;
	otoz.active = true;
	assert(not otoz.fixed);
	
	ValFix pinf = ValFix(), ninf = ValFix(); // @
	if (datatype() == F32) {
		pinf = ValFix( VariantType(+std::numeric_limits<Ctype< F32 >>::infinity(),datatype()) );
		ninf = ValFix( VariantType(-std::numeric_limits<Ctype< F32 >>::infinity(),datatype()) );
	}
	if (datatype() == F64) {
		pinf = ValFix( VariantType(+std::numeric_limits<Ctype< F64 >>::infinity(),datatype()) );
		ninf = ValFix( VariantType(-std::numeric_limits<Ctype< F64 >>::infinity(),datatype()) );
	}
	auto isFinite = [](ValFix vf){ return !vf.max.isInf() && !vf.min.isInf() && !vf.mean.isNan(); };
	auto isNumber = [](ValFix vf){ return !vf.max.isNan() && !vf.min.isNan() && !vf.mean.isNan(); };

	auto lvf = hash.find({left(),coord})->second;
	auto rvf = hash.find({right(),coord})->second;
	auto lval = lvf.value;
	auto lfix = lvf.fixed;
	auto rval = rvf.value;
	auto rfix = rvf.fixed;

	// If both values are fixed, just comptute normally
	if (lfix && rfix) {
		hash[{node,coord}] = ValFix(type.apply(lval,rval));
		return;
	}

	// Else, if some does not presents statistics, nothing to do
	if (not lvf.active || not rvf.active) {
		hash[{node,coord}] = ValFix();
		return;
	}

	// Else (both active / perhaps one fixed), try arithmetic simplifications
	switch (type.get()) {
		case NONE_BINARY: assert(0);
		case ADD: {
			vf.min = lvf.min + rvf.min;
			vf.max = lvf.max + rvf.max;
			vf.mean = (lvf.mean + rvf.mean) / 2;
			vf.std = (lvf.std + rvf.std) / 2; // @
			vf.active = true;
			assert(not vf.fixed);
			// @ and the cases related to inf,nan?
			break;
		}
		case SUB:  {
			vf.min = lvf.min - rvf.max;
			vf.max = lvf.max - rvf.min;
			vf.mean = lvf.mean - rvf.mean; // @
			vf.std = (lvf.std + rvf.std) / 2; // @
			vf.active = true;
			assert(not vf.fixed);
			break;
		}
		case MUL: {
			if (lfix && lval.isZero() && isFinite(rvf)) {
				vf = zero;
			} else if (rfix && rval.isZero() && isFinite(lvf)) {
				vf = zero;
			} else if (isFinite(lvf) && isFinite(rvf)) {
				auto a = lvf.min * rvf.min;
				auto b = lvf.min * rvf.max;
				auto c = lvf.max * rvf.min;
				auto d = lvf.max * rvf.max;
				vf.min = _min(_min(a,b),_min(c,d));
				vf.max = _max(_max(a,b),_max(c,d));
				vf.mean = (vf.min + vf.max) / 2; // @
				vf.std = (vf.max - vf.min) / 4; // @
				vf.active = true;
				assert(not vf.fixed);
			} else if (isNumber(lvf) && isNumber(rvf)) {
				vf.min = ninf.value;
				vf.max = pinf.value;
				vf.mean = zero.value;
				vf.mean = pinf.value;
				vf.active = true;
				assert(not vf.fixed);
			} else if (true) {
				assert(0); // @ other cases related to nan?
			}
			break;
		}
		case DIV: {
			if (lfix && lval.isZero() && isFinite(rvf)) {
				vf = zero;
			} else if (isFinite(lvf) && isFinite(rvf)) {
				auto a = lvf.min / rvf.min;
				auto b = lvf.min / rvf.max;
				auto c = lvf.max / rvf.min;
				auto d = lvf.max / rvf.max;
				vf.min = _min(_min(a,b),_min(c,d));
				vf.max = _max(_max(a,b),_max(c,d));
				vf.mean = (vf.min + vf.max) / 2; // @
				vf.std = (vf.max - vf.min) / 4; // @
				vf.active = true;
				assert(not vf.fixed);
			} else if (isNumber(lvf) && isNumber(rvf)) {
				vf.min = ninf.value;
				vf.max = pinf.value;
				vf.mean = zero.value;
				vf.mean = pinf.value;
				vf.active = true;
				assert(not vf.fixed);
			} else if (true) {
				assert(0); // @ other cases related to nan?
			}
			break;
		}
		case MOD: break;
		case EQ: vf = otoz; break; // inf == inf, nan == nan?
		case NE: vf = otoz; break; // inf != inf, nan != nan?
		case LT: {
			if (type.apply(lvf.max,rvf.min)) {
				vf = one;
			} else if (BinaryType(LE).apply(rvf.max,lvf.min)) {
				vf = zero;
			} else {
				vf = zero;
				vf.max = one.max;
				vf.active = true;
				vf.fixed = false;
			}
			break; // inf, nan?
		}
		case GT: {
			if (type.apply(lvf.min,rvf.max)) {
				vf = one;
			} else if (BinaryType(GE).apply(rvf.min,lvf.max)) {
				vf = zero;
			} else {
				vf = zero;
				vf.max = one.max;
				vf.active = true;
				vf.fixed = false;
			}
			break; // inf, nan?
		}
		case LE: {
			if (type.apply(lvf.max,rvf.min)) {
				vf = one;
			} else if (BinaryType(LT).apply(rvf.max,lvf.min)) {
				vf = zero;
			} else {
				vf = zero;
				vf.max = one.max;
				vf.active = true;
				vf.fixed = false;
			}
			break; // inf, nan?
		}
		case GE: {
			if (type.apply(lvf.min,rvf.max)) {
				vf = one;
			} else if (BinaryType(GT).apply(rvf.min,lvf.max)) {
				vf = zero;
			} else {
				vf = zero;
				vf.max = one.max;
				vf.active = true;
				vf.fixed = false;
			}
			break; // inf, nan?
		}
		case AND: vf = otoz; break;
		case OR: vf = otoz; break;
		case bAND: vf = otoz; break; // @
		case bOR: vf = otoz; break; // @
		case bXOR: vf = otoz; break; // @
		case SHL: break;
		case SHR: break;
		case MAX2: {
			if (lvf.min.isInf() && lvf.min.isPos() && !rvf.mean.isNan()) {
				vf = pinf;
			} else if (rvf.min.isInf() && rvf.min.isPos() && !lvf.mean.isNan()) {
				vf = pinf;
			} else if (lvf.max.isInf() && lvf.min.isNeg() && !rvf.mean.isNan()) {
				vf = rvf;
			} else if (rvf.max.isInf() && rvf.min.isNeg() && !lvf.mean.isNan()) {
				vf = lvf;
			} else if (isFinite(lvf) && isFinite(rvf)) {
				vf = otoz;
			}
			break;
		}
		case MIN2: {
			if (lvf.max.isInf() && lvf.min.isNeg() && !rvf.mean.isNan()) {
				vf = ninf;
			} else if (rvf.max.isInf() && rvf.min.isNeg() && !lvf.mean.isNan()) {
				vf = ninf;
			} else if (lvf.min.isInf() && lvf.min.isPos() && !rvf.mean.isNan()) {
				vf = rvf;
			} else if (rvf.min.isInf() && rvf.min.isPos() && !lvf.mean.isNan()) {
				vf = lvf;
			} else if (isFinite(lvf) && isFinite(rvf)) {
				vf = otoz;
			}
			break;
		}
		case ATAN2: break;
		case POW: break;
		case HYPOT: break;
		case FMOD: break;
	}

	assert(vf.active);
	assert(vf.max >= vf.min);

	hash[{node,coord}] = vf;
}

} } // namespace map::detail
