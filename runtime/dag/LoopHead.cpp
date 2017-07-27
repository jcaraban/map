/**
 * @file	LoopHead.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "LoopHead.hpp"
#include "LoopCond.hpp"
#include "../Runtime.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

LoopHead::Content::Content(LoopHead *node) {
	prev = node->prev();
	loop = node->loop();
}

bool LoopHead::Content::operator==(const Content& k) const {
	return (prev==k.prev && loop==k.loop);
}

std::size_t LoopHead::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<LoopCond*>()(k.loop);
}

// Factory

Node* LoopHead::Factory(Node *prev) {
	assert(prev != nullptr);

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	GroupSize gs = prev->groupsize();
	MetaData meta(ds,dt,mo,bs,gs);

	meta.stream_dir = prev->streamdir(); // @

	return new LoopHead(meta,prev);
}

Node* LoopHead::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new LoopHead(this,other_to_this);
}

// Constructors

LoopHead::LoopHead(const MetaData &meta, Node *prev)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->file = prev->file;
	
	owner_loop = nullptr; // 'head' knows who its 'loop' is
	twin_tail = nullptr; // 'head' might have a twin 'tail'
	
	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

LoopHead::LoopHead(const LoopHead *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->owner_loop = nullptr; // filled later by 'loop', because it does not live yet
	this->twin_tail = nullptr; // might be filled later by a 'tail', if a twin exists
}

LoopHead::~LoopHead() {
	// Notifies its 'loop' about the deletion
	if (owner_loop != nullptr)
		remove_value(this,owner_loop->head_list);
}

// Methods

void LoopHead::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string LoopHead::shortName() const {
	return "LoopHead";
}

std::string LoopHead::longName() const {
	std::string str = "LoopHead {" + std::to_string(prev()->id) + "}";
	return str;
}

std::string LoopHead::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	return sign;
}

LoopCond* LoopHead::loop() const {
	return owner_loop;
}

Node* LoopHead::prev() const {
	return prev_list[0];
}

// Compute

void LoopHead::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(numdim() == D0);
	hash[this] = hash.find(prev())->second;
}

void LoopHead::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = hash.find({prev(),coord})->second;
}

} } // namespace map::detail
