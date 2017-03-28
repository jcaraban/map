/**
 * @file	Conditional.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: e.g. con(local,focal,spread), it might be efficient to not fuse a condition and check
 *       if a block fully evaluates (ie all cells) to true/false, then computing the spread for
 *       that block would be unnecesary. However, the runtime support required gets too complex.
 */

#include "Conditional.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Conditional::Key::Key(Conditional *node) {
	cond = node->cond();
	lprev = node->left();
	rprev = node->right();
}

bool Conditional::Key::operator==(const Key& k) const {
	return (cond==k.cond && lprev==k.lprev && rprev==k.rprev);
}

std::size_t Conditional::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.cond) ^ std::hash<Node*>()(k.lprev) ^ std::hash<Node*>()(k.rprev);
}

// Factory

Node* Conditional::Factory(Node *cond, Node *lhs, Node *rhs) {
	assert(cond != nullptr && lhs != nullptr && rhs != nullptr);

	NumDim dim, cdim = cond->numdim(), ldim = lhs->numdim(), rdim = rhs->numdim();
	DataSize ds, cds = cond->datasize(), lds = lhs->datasize(), rds = rhs->datasize();
	DataType dt, cdt = cond->datatype(), ldt = lhs->datatype(), rdt = rhs->datatype();
	MemOrder mo, cmo = cond->memorder(), lmo = lhs->memorder(), rmo = rhs->memorder();
	BlockSize bs, cbs = cond->blocksize(), lbs = lhs->blocksize(), rbs = rhs->blocksize();

	/****/ if (cdim == D0) {
		if (ldim == D0) {
			dim=rdim, ds=rds, mo=rmo, bs=rbs;
		} else if (rdim == D0) {
			dim=ldim, ds=lds, mo=lmo, bs=lbs;	
		} else {
			assert( ldim==rdim && all(lds==rds) && lmo==rmo && all(lbs==rbs) );
			dim=ldim, ds=lds, mo=lmo, bs=lbs;
		}
	} else if (ldim == D0) {
		if (cdim == D0) {
			dim=rdim, ds=rds, mo=rmo, bs=rbs;
		} else if (rdim == D0) {
			dim=cdim, ds=cds, mo=cmo, bs=cbs;
		} else {
			assert( cdim==rdim && all(cds==rds) && cmo==rmo && all(cbs==rbs) );
			dim=cdim, ds=cds, mo=cmo, bs=cbs;
		}
	} else if (rdim == D0) {
		if (cdim == D0) {
			dim=ldim, ds=lds, mo=lmo, bs=lbs;
		} else if (ldim == D0) {
			dim=cdim, ds=cds, mo=cmo, bs=cbs;
		} else {
			assert( cdim==ldim && all(cds==lds) && cmo==lmo && all(cbs==lbs) );
			dim=cdim, ds=cds, mo=cmo, bs=cbs;
		}
	} else {
		assert(cdim==ldim && ldim==rdim);
		assert(all(cds==rds) && all(lds==rds));
		assert(cmo==lmo && lmo==rmo);
		assert(all(cbs==lbs) && all(lbs==rbs));
		dim=cdim, ds=cds, mo=cmo, bs=cbs;
	}

	assert(ldt == rdt); 
	dt = ldt; // @ if cdim == D0, evaluate 'cond' and assign the type of 'lhs' or 'rhs'

	MetaData meta(ds,dt,mo,bs);

	// @ identity functionality would go here

	return new Conditional(meta,cond,lhs,rhs);
}

Node* Conditional::clone(std::unordered_map<Node*,Node*> other_to_this) {
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
}

Conditional::Conditional(const Conditional *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Conditional::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Conditional::getName() const {
	return "Conditional";
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
/*
Node*& Conditional::cond() {
	return prev_list[0];
}
*/
Node* Conditional::cond() const {
	return prev_list[0];
}
/*
Node*& Conditional::left() {
	return prev_list[1]; // second element
}
*/
Node* Conditional::left() const {
	return prev_list[1]; // second element
}
/*
Node*& Conditional::right() {
	return prev_list[2]; // third element
}
*/
Node* Conditional::right() const {
	return prev_list[2]; // third element
}

} } // namespace map::detail
