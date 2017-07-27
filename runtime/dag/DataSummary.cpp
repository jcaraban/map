/**
 * @file	DataSummary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "DataSummary.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

DataSummary::Content::Content(DataSummary *node) {
	prev = node->prev();
	type = node->type;
}

bool DataSummary::Content::operator==(const Content& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t DataSummary::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* DataSummary::Factory(Node *prev, ReductionType type) {
	assert(prev != nullptr);
	assert(prev->numdim() != D0);

	DataSize ds = prev->numdim().unitVec();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->numdim().unitVec();
	GroupSize gs = prev->numdim().unitVec();
	MetaData meta(ds,dt,mo,bs,gs);

	return new DataSummary(meta,prev,type);
}

Node* DataSummary::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new DataSummary(this,other_to_this);
}

// Constructors

DataSummary::DataSummary(const MetaData &meta, Node *prev, ReductionType type) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->type = type;
	//this->value = type.neutral(datatype());
	
	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true); // @ shall be something else ?
}

DataSummary::DataSummary(const DataSummary *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void DataSummary::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string DataSummary::shortName() const {
	return "DataSummary";
}

std::string DataSummary::longName() const {
	std::string str = "DataSummary {" + std::to_string(prev()->id) + "}";
	return str;
}

std::string DataSummary::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* DataSummary::prev() const {
	return prev_list[0];
}

// Compute

void DataSummary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();

	auto prev = hash.find({node->prev(),coord})->second;
	if (prev.fixed) {
		switch (node->type.get()) {
			case SUM:  vf = ValFix( BinaryType(MUL).apply( prev.value , prod(blocksize()) ) ); break;
			case PROD: vf = ValFix( BinaryType(POW).apply( prev.value , prod(blocksize()) ) ); break;
			case rAND: vf = ValFix(prev.value); break;
			case rOR:  vf = ValFix(prev.value); break;
			case MAX:  vf = ValFix(prev.value); break;
			case MIN:  vf = ValFix(prev.value); break;
			default: assert(0);
		}
	}
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
