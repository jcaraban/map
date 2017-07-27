/**
 * @file	Temporal.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Temporal.hpp"
#include "../visitor/Visitor.hpp"


namespace map { namespace detail {

// Factory

Node* Temporal::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Temporal(this,other_to_this);
}

// Constructors

Temporal::Temporal(const MetaData &meta) :
	Node(meta)
{ }

Temporal::Temporal(const Temporal *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Temporal::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Temporal::shortName() const {
	return "Temporal";
}

std::string Temporal::longName() const {
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
