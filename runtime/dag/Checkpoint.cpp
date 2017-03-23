/**
 * @file	Checkpoint.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Checkpoint.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Checkpoint::Key::Key(Checkpoint *node) {
	prev = node->prev();
}

bool Checkpoint::Key::operator==(const Key& k) const {
	return (prev==k.prev);
}

std::size_t Checkpoint::Hash::operator()(const Key& k) const {
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

Node* Checkpoint::clone(NodeList new_prev_list, NodeList new_back_list) {
	return new Checkpoint(this,new_prev_list,new_back_list);
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

Checkpoint::Checkpoint(const Checkpoint *other, NodeList new_prev_list, NodeList new_back_list)
	: IONode(other,new_prev_list,new_back_list) // because of virtual inheritance
	, OutInNode()
{ }

// Methods

void Checkpoint::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Checkpoint::getName() const {
	return "Checkpoint";
}

std::string Checkpoint::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

} } // namespace map::detail
