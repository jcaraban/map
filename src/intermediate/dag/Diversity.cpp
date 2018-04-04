/**
 * @file	Diversity.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Diversity.hpp"
#include "../../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Diversity::Content::Content(Diversity *node) {
	prev_list = node->prev_list;
	type = node->type;
}

bool Diversity::Content::operator==(const Content& k) const {
	if (prev_list.size() != k.prev_list.size())
		return false;
	bool b = true;
	for (int i=0; i<prev_list.size(); i++)
		b &= prev_list[i] == k.prev_list[i];
	b &= type == k.type;
	return b;
}

std::size_t Diversity::Hash::operator()(const Content& k) const {
	size_t h = 0;
	for (auto &prev : k.prev_list)
		h ^= std::hash<Node*>()(prev);
	h ^= std::hash<int>()(k.type.get());
	return h;
}

Node* Diversity::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Diversity(this,other_to_this);
}

// Factory

Node* Diversity::Factory(NodeList prev_list, DiversityType type) {
	assert(prev_list.size() > 1);

	int i = 0;
	DataSize ds = prev_list[i]->datasize();
	DataType dt = prev_list[i]->datatype();
	MemOrder mo = prev_list[i]->memorder();
	BlockSize bs = prev_list[i]->blocksize();
	GroupSize gs = prev_list[i]->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	for (auto prev : prev_list) {
		assert(meta == prev->metadata());
	}

	if (type == VARI) { // Variety always output an integer
		meta.data_type = S8;
	}
	
	return new Diversity(meta,prev_list,type);
}

// Constructors

Diversity::Diversity(const MetaData &meta, NodeList prev_list, DiversityType type)
	: Node(meta)
{
	this->prev_list = prev_list;
	for (auto prev : prev_list)
		prev->addNext(this);

	this->type = type;

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Diversity::Diversity(const Diversity *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void Diversity::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Diversity::shortName() const {
	return "Diversity";
}

std::string Diversity::longName() const {
	std::string str = "Diversity {";
	for (auto prev : prevList())
		str += std::to_string(prev->id);
	return str + "}";
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

void Diversity::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	std::vector<VariantType> varvec;
	for (auto prev : prevList())
		varvec.push_back( hash.find(prev)->second );
	hash[this] = type.apply(varvec);
}

void Diversity::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	ValFix vf = ValFix();
	ValFix vf0 = ValFix(VariantType(0,datatype()));
	
	for (auto prev : prevList()) {
		auto p = hash[{prev,coord}];
		if (!p.fixed) {
			hash[{this,coord}] = ValFix();
			return;
		}
	}
	std::vector<VariantType> vec;
	for (auto prev : prevList()) {
		auto p = hash[{prev,coord}];
		vec.push_back(p.value);
	}
	hash[{this,coord}] = ValFix(type.apply(vec));
}

} } // namespace map::detail
