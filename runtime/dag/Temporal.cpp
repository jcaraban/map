/**
 * @file	Temporal.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Temporal.hpp"
#include "../visitor/Visitor.hpp"


namespace map { namespace detail {

Temporal::Temporal(const MetaData &meta) :
	Node(meta)
{ }

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
