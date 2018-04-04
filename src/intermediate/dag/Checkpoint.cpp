/**
 * @file	Checkpoint.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Checkpoint.hpp"
#include "../../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Checkpoint::Content::Content(Checkpoint *node) {
	prev = node->prev();
}

bool Checkpoint::Content::operator==(const Content& k) const {
	return (prev==k.prev);
}

std::size_t Checkpoint::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Checkpoint::Factory(Node *prev) {
	assert(prev != nullptr);

	// Creates a temporal binary file
	SharedFile tmp_file = SharedFile( IFile::Factory(prev) );
	
	// Creates and evaluates the Checkpoint node
	return new Checkpoint(prev,tmp_file);
}

Node* Checkpoint::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Checkpoint(this,other_to_this);
}

// Constructors

Checkpoint::Checkpoint(Node *prev, SharedFile tmp_file)
	: IONode(tmp_file,OutputNodeFlag()) // because of virtual inheritance
	, OutInNode(prev,tmp_file)
{
	// 'next' of 'prev' now link to checkpoint
	for (auto next : prev->nextList())
		if (next != this)
			next->updatePrev(prev,this);
assert(!"updatePrev was changed");
	// 'checkpoint' links to every 'next'
	for (auto next : prev->nextList())
		if (next != this)
			this->addNext(next);

	// 'prev' looses the links to its 'next'
	prev->ref -= prev->next_list.size();
	prev->next_list.clear();
	// in exchange of a link to 'check'
	prev->addNext(this);
}

Checkpoint::Checkpoint(const Checkpoint *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: IONode(other,other_to_this) // because of virtual inheritance
	, OutInNode()
{ }

// Methods

void Checkpoint::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Checkpoint::shortName() const {
	return "Checkpoint";
}

std::string Checkpoint::longName() const {
	std::string str = "Checkpoint {" + std::to_string(prev()->id) + "}";
	return str;
}

std::string Checkpoint::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

// Compute

void Checkpoint::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;

	hash[{node,coord}] = hash.find({node->prev(),coord})->second;
}

} } // namespace map::detail
