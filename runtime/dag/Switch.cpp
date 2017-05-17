/**
 * @file	Switch.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Switch.hpp"
#include "LoopHead.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Switch::Content::Content(Switch *node) {
	cond = node->cond();
	prev = node->prev();
}

bool Switch::Content::operator==(const Content& k) const {
	return (cond==k.cond && prev==k.prev);
}

std::size_t Switch::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.cond) ^ std::hash<Node*>()(k.prev);
}

// Switch

Node* Switch::Factory(Node *cond, Node *prev) {
	assert(cond != nullptr);
	assert(prev != nullptr);

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	GroupSize gs = prev->groupsize();
	
	MetaData meta(ds,dt,mo,bs,gs);

	return new Switch(meta,cond,prev);
}

Node* Switch::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Switch(this,other_to_this);
}

// Constructors

Switch::Switch(const MetaData &meta, Node *cond, Node *prev)
	: Node(meta)
{
	prev_list.reserve(2);
	this->addPrev(cond); // condition first, closes the loop before moving up
	this->addPrev(prev);
	cond->addNext(this);
	prev->addNext(this);

	this->next_true = NodeList(); // Added during loop/if-assembly
	this->next_false = NodeList(); // Added during loop/if-assembly

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Switch::Switch(const Switch *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->next_true = other->next_true;
	this->next_false = other->next_false;
}

// Methods

void Switch::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Switch::getName() const {
	return "Switch";
}

std::string Switch::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

Node* Switch::cond() const {
	return prev_list[0];
}

Node* Switch::prev() const {
	return prev_list[1];
}

const NodeList& Switch::trueList() const {
	assert(next_true.size() + next_false.size() == next_list.size());
	return next_true;
}

const NodeList& Switch::falseList() const {
	assert(next_true.size() + next_false.size() == next_list.size());
	return next_false;
}

void Switch::addTrue(Node *node) {
	auto it = std::find(next_list.begin(),next_list.end(),node);
	assert(it != next_list.end());
	it = std::find(next_false.begin(),next_false.end(),node);
	assert(it == next_false.end());

	next_true.push_back(node);
}

void Switch::addFalse(Node *node) {
	auto it = std::find(next_list.begin(),next_list.end(),node);
	assert(it != next_list.end());
	it = std::find(next_true.begin(),next_true.end(),node);
	assert(it == next_true.end());

	next_false.push_back(node);
}

// Compute 

void Switch::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	auto *node = this;

	auto pval = hash.find(node->prev())->second;
	hash[node] = pval;
}

void Switch::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	auto prev = hash.find({node->prev(),coord})->second;
	hash[{node,coord}] = ValFix(prev.value,prev.fixed);
}

} } // namespace map::detail
