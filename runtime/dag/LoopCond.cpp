/**
 * @file	LoopCond.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LoopCond.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LoopCond::Content::Content(LoopCond *node) {
	prev = node->prev();
	head_list = node->headList();
	tail_list = node->tailList();
}

bool LoopCond::Content::operator==(const Content& k) const {
	if (head_list.size() != k.head_list.size())
		return false;
	if (tail_list.size() != k.tail_list.size())
		return false;
	bool b = prev == k.prev;
	for (int i=0; i<head_list.size(); i++)
		b &= head_list[i] == k.head_list[i];
	for (int i=0; i<tail_list.size(); i++)
		b &= tail_list[i] == k.tail_list[i];
	return b;
}

std::size_t LoopCond::Hash::operator()(const Content& k) const {
	size_t h = std::hash<Node*>()(k.prev);
	for (auto &node : k.head_list)
		h ^= std::hash<Node*>()(node);
	for (auto &node : k.tail_list)
		h ^= std::hash<Node*>()(node);
	return h;
}

// Factory

Node* LoopCond::Factory(Node *prev) {
	assert(prev != nullptr);

	DataSize ds = prev->datasize();
	DataType dt = U8; // @
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	GroupSize gs = prev->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);
	
	return new LoopCond(meta,prev);
}

Node* LoopCond::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new LoopCond(this,other_to_this);
}

// Constructors

LoopCond::LoopCond(const MetaData &meta, Node *prev)
	: Node(meta)
{
	this->prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

LoopCond::LoopCond(const LoopCond *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->head_list.reserve(other->head_list.size());
	this->tail_list.reserve(other->tail_list.size());

	for (auto other_head : other->head_list) {
		if (other_to_this.find(other_head) == other_to_this.end())
			continue; // Some living Heads might not be on the cloned list
		Node *this_head = other_to_this.find(other_head)->second;
		this->head_list.push_back( dynamic_cast<LoopHead*>(this_head) );
		this->head_list.back()->owner_loop = this;  // the 'head' clone is completed now
	}
	
	// The 'tail' nodes will link back later and complete the network
}

LoopCond::~LoopCond() {
	for (auto head : head_list)
		head->owner_loop = nullptr;
	for (auto tail : tail_list)
		tail->owner_loop = nullptr;
}

// Methods

void LoopCond::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LoopCond::getName() const {
	return "LoopCond";
}

std::string LoopCond::signature() const {
	std::string sign = "";
	sign += classSignature();
	for (auto prev : prev_list) {
		sign += prev->numdim().toString();
		sign += prev->datatype().toString();
	}
	return sign;
}

const HeadList& LoopCond::headList() const {
	return head_list;
}

const TailList& LoopCond::tailList() const {
	return tail_list;
}

Node* LoopCond::prev() const {
	return prev_list[0];
}

// Features

//bool LoopCond::isReduction() const {
//	return numdim() != D0;
//}

ReductionType LoopCond::reductype() const {
	return rOR;
}

VariantType LoopCond::initialValue() const {
	return ReductionType(rOR).neutral(datatype());
}

// Compute

void LoopCond::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto val = hash.find({prev(),coord})->second.value;
	hash[{this,coord}] = ValFix(val.convert(datatype()));
}

} } // namespace map::detail
