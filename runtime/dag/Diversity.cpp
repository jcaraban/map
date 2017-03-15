/**
 * @file	Diversity.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Diversity.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Diversity::Key::Key(Diversity *node) {
	prev_list = node->prev_list;
	type = node->type;
}

bool Diversity::Key::operator==(const Key& k) const {
	if (prev_list.size() != k.prev_list.size())
		return false;
	bool b = true;
	for (int i=0; i<prev_list.size(); i++)
		b &= prev_list[i] == k.prev_list[i];
	b &= type == k.type;
	return b;
}

std::size_t Diversity::Hash::operator()(const Key& k) const {
	size_t h = 0;
	for (auto &prev : k.prev_list)
		h ^= std::hash<Node*>()(prev);
	h ^= std::hash<int>()(k.type.get());
	return h;
}

Node* Diversity::clone(NodeList new_prev_list) {
	return new Diversity(this,new_prev_list);
}

// Factory

Node* Diversity::Factory(NodeList prev_list, DiversityType type) {
	assert(prev_list.size() > 1);

	int i = 0;
	DataSize ds = prev_list[i]->datasize();
	DataType dt = prev_list[i]->datatype();
	MemOrder mo = prev_list[i]->memorder();
	BlockSize bs = prev_list[i]->blocksize();

	for (auto prev : prev_list) {
		assert( all(ds == prev->datasize()) );
		assert( dt == prev->datatype() );
		assert( mo == prev->memorder() );
		assert( all(bs == prev->blocksize()) );
	}

	if (type == VARI) { // Variety always output an integer
		dt = S8;
	}

	MetaData meta(ds,dt,mo,bs);
	
	return new Diversity(meta,prev_list,type);
}

// Constructors

Diversity::Diversity(const MetaData &meta, NodeList prev_list, DiversityType type)
	: Node(meta)
{
	this->prev_list = prev_list;
	this->type = type;

	for (auto prev : prev_list)
		prev->addNext(this);
}

Diversity::Diversity(const Diversity *other, NodeList new_prev_list)
	: Node(other,new_prev_list)
{
	this->type = other->type;
}

// Methods

void Diversity::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Diversity::getName() const {
	return "Diversity";
}

std::string Diversity::signature() const {
	std::string sign = "";
	sign += classSignature();
	for (auto prev : prev_list) {
		sign += prev->numdim().toString();
		sign += prev->datatype().toString();
	}
	sign += type.toString();
	return sign;
}

Node*& Diversity::prev(int i) {
	return prev_list[i];
}

const Node* Diversity::prev(int i) const {
	return prev_list[i];
}

} } // namespace map::detail
