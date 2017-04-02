/**
 * @file	Merge.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: not restricting the patterns could work, but would inhibit fusion in those cases
 */

#include "Merge.hpp"
#include "Constant.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Merge::Key::Key(Merge *node) {
	lprev = node->left();
	rprev = node->right();
}

bool Merge::Key::operator==(const Key& k) const {
	return (lprev==k.lprev && rprev==k.rprev);
}

std::size_t Merge::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.lprev) ^ std::hash<Node*>()(k.rprev);
}

// Factory

Node* Merge::Factory(Node *lhs, Node *rhs) {
	assert(lhs != nullptr);
	assert(rhs != nullptr);

	NumDim dim, ldim = lhs->numdim(), rdim = rhs->numdim();
	DataSize ds, lds = lhs->datasize(), rds = rhs->datasize();
	DataType dt, ldt = lhs->datatype(), rdt = rhs->datatype();
	MemOrder mo, lmo = lhs->memorder(), rmo = rhs->memorder();
	BlockSize bs, lbs = lhs->blocksize(), rbs = rhs->blocksize();

	if (ldim == D0) {
		dim=rdim, ds=rds, mo=rmo, bs=rbs;
	} else if (rdim == D0) {
		dim=ldim, ds=lds, mo=lmo, bs=lbs;
	} else {
		assert( ldim==rdim && all(lds==rds) && lmo==rmo && all(lbs==rbs) );
		dim=ldim, ds=lds, mo=lmo, bs=lbs;
	}
	dt = DataType( promote(ldt,rdt) );	

	MetaData meta(ds,dt,mo,bs);

	return new Merge(meta,lhs,rhs);
}

Node* Merge::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new Merge(this,other_to_this);
}

// Constructors

Merge::Merge(const MetaData &meta, Node *lprev, Node *rprev)
	: Node(meta)
{
	//this->pat = lprev->pattern() + rprev->pattern();

	if (this->id > rprev->id) // Merge of an structured if-else
	{
		assert(0); // @ not this yet
		
		prev_list.reserve(2);
		this->addPrev(lprev);
		this->addPrev(rprev);
		
		lprev->addNext(this);
		rprev->addNext(this);
	}
	else if (this->id < rprev->id) // Merge of a structure while loop
	{
		prev_list.reserve(1);
		this->addPrev(lprev);
		forw_list.reserve(1);
		this->addForw(rprev);

		lprev->addNext(this);
		rprev->addBack(this);
	}
	assert(lprev->id != rprev->id);
}

Merge::Merge(const Merge *other, std::unordered_map<Node*,Node*> other_to_this)
	: Node(other,other_to_this)
{
	//this->pat = other->pat;
}

// Methods

void Merge::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Merge::getName() const {
	return "Merge";
}

std::string Merge::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += left()->numdim().toString();
	sign += left()->datatype().toString();
	sign += right()->numdim().toString();
	sign += right()->datatype().toString();
	return sign;
}
/*
Node*& Merge::left() {
	return prev_list[0]; // first element
}
*/
Node* Merge::left() const {
	return prev_list[0]; // first element
}
/*
Node*& Merge::right() {
	return prev_list[1]; // second element
}
*/
Node* Merge::right() const {
	assert(prev_list.size() + forw_list.size() == 2);
	return (prev_list.size() == 2) ? prev_list[1] : forw_list[0];
}

Pattern Merge::pattern() const {
	return MERGE;
}

} } // namespace map::detail
