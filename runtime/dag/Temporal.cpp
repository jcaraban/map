/**
 * @file	Temporal.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Temporal.hpp"
#include "../visitor/Visitor.hpp"


namespace map { namespace detail {

// Factory

Node* Temporal::clone(NodeList new_prev_list) {
	return new Temporal(this,new_prev_list);
}

// Constructors

Temporal::Temporal(const MetaData &meta) :
	Node(meta)
{ }

Temporal::Temporal(const Temporal *other, NodeList new_prev_list)
	: Node(other,new_prev_list)
{ }

// Methods

void Temporal::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Temporal::getName() const {
	return "Temporal";
}

std::string Temporal::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += numdim().toString();
	sign += datatype().toString();
	return sign;
}

} } // namespace map::detail
