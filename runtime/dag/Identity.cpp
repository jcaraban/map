/**
 * @file	Identity.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Identity.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Identity::Content::Content(Identity *node) {
	prev = node->prev();
}

bool Identity::Content::operator==(const Content& k) const {
	return (prev==k.prev);
}

std::size_t Identity::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Identity::Factory(Node *prev) {
	assert(prev != nullptr);
	return new Identity(prev->metadata(),prev);
}

Node* Identity::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Identity(this,other_to_this);
}

// Constructors

Identity::Identity(const MetaData &meta, Node *prev)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	
	prev->addNext(this);
}

Identity::Identity(const Identity *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Identity::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Identity::getName() const {
	return "Identity";
}

std::string Identity::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	return sign;
}

Node* Identity::prev() const {
	return prev_list[0];
}

// Compute

void Identity::computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash) {
	assert(numdim() == D0);
	Coord coord = {0,0};
	auto *node = this;

	auto pval = hash.find({node->prev(),coord})->second;
	hash[{node,coord}] = pval;
}

void Identity::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = hash.find({prev(),coord})->second;
}

} } // namespace map::detail
