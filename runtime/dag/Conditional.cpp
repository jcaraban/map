/**
 * @file	Conditional.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: e.g. con(local,focal,radial), it might be efficient to not fuse a condition and check
 *       if a block fully evaluates (ie all cells) to true/false, then computing the radial for
 *       that block would be unnecesary. However, the required runtime support gets too complex.
 */

#include "Conditional.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Conditional::Content::Content(Conditional *node) {
	cond = node->cond();
	lprev = node->left();
	rprev = node->right();
}

bool Conditional::Content::operator==(const Content& k) const {
	return (cond==k.cond && lprev==k.lprev && rprev==k.rprev);
}

std::size_t Conditional::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.cond) ^ std::hash<Node*>()(k.lprev) ^ std::hash<Node*>()(k.rprev);
}

// Factory

Node* Conditional::Factory(Node *cond, Node *lhs, Node *rhs) {
	assert(cond != nullptr && lhs != nullptr && rhs != nullptr);

	DataType data_type;
	MemOrder mem_order;
	DataShape shape;
	DataShape cshp = cond->metadata().getDataShape();
	DataShape lshp = lhs->metadata().getDataShape();
	DataShape rshp = rhs->metadata().getDataShape();

	if (cond->numdim() == D0) {
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
	} else if (lhs->numdim() == D0) {
		if (cond->numdim() == D0) {
			shape = rshp;
			mem_order = rhs->memorder();
		} else if (rhs->numdim() == D0) {
			shape = cshp;
			mem_order = cond->memorder();
		} else {
			assert(cshp == rshp);
			assert(cond->memorder() == rhs->memorder());
			shape = cshp;
			mem_order = rhs->memorder();
		}
	} else if (rhs->numdim() == D0) {
		if (cond->numdim() == D0) {
			shape = lshp;
			mem_order = lhs->memorder();
		} else if (lhs->numdim() == D0) {
			shape = cshp;
			mem_order = cond->memorder();
		} else {
			assert(cshp == lshp);
			shape = cshp;
			mem_order = cond->memorder();
		}
	} else {
		assert(cshp == lshp && lshp == rshp);
		assert(cond->memorder() == lhs->memorder());
		assert(lhs->memorder() == rhs->memorder());
		shape = cshp;
		mem_order = cond->memorder();
	}

	assert(lhs->datatype() == rhs->datatype()); 
	data_type = lhs->datatype(); // @ if cdim == D0, evaluate 'cond' and assign the type of 'lhs' or 'rhs'

	MetaData meta(shape.data_size,data_type,mem_order,shape.block_size,shape.group_size);

	// @ identity functionality would go here ?

	return new Conditional(meta,cond,lhs,rhs);
}

Node* Conditional::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Conditional(this,other_to_this);
}

// Constructors

Conditional::Conditional(const MetaData &meta, Node *cond, Node *lprev, Node *rprev)
	: Node(meta)
{
	prev_list.reserve(3);
	this->addPrev(cond);  // [0]
	this->addPrev(lprev); // [1]
	this->addPrev(rprev); // [2]
	
	cond->addNext(this);
	lprev->addNext(this);
	rprev->addNext(this);

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Conditional::Conditional(const Conditional *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Conditional::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Conditional::shortName() const {
	return "Conditional";
}

std::string Conditional::longName() const {
	auto cid = std::to_string(cond()->id);
	auto lid = std::to_string(left()->id);
	auto rid = std::to_string(right()->id);
	std::string str = "Conditional {" + cid + "," + lid + "," + rid + "}";
	return str;
}

std::string Conditional::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += cond()->numdim().toString();
	sign += cond()->datatype().toString();
	sign += left()->numdim().toString();
	sign += left()->datatype().toString();
	sign += right()->numdim().toString();
	sign += right()->datatype().toString();
	return sign;
}

Node* Conditional::cond() const {
	return prev_list[0];
}

Node* Conditional::left() const {
	return prev_list[1]; // second element
}

Node* Conditional::right() const {
	return prev_list[2]; // third element
}

void Conditional::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	auto cval = hash.find(cond())->second;
	auto lval = hash.find(left())->second;
	auto rval = hash.find(right())->second;
	hash[this] = cval ? lval : rval;
}

void Conditional::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();

	auto cond = hash[{node->cond(),coord}];
	auto left = hash[{node->left(),coord}];
	auto right = hash[{node->right(),coord}];

	if (cond.fixed) {
		if (cond.value) {
			if (left.fixed) 
				vf = ValFix(left.value);
		} else {
			if (right.fixed)
				vf = ValFix(right.value);
		}
	}
	hash[{node,coord}] = vf;

	// TODO: if 'cond' is fixed but 'left'/'right' are not, still one side might be ignored
}

} } // namespace map::detail
