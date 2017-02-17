/**
 * @file	FlowType.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "FlowType.hpp"
#include <cassert>


namespace map { namespace detail {

FlowType::FlowType()
	: type(NONE_FLOW)
{ }

FlowType::FlowType(FlowEnum type) {
	assert(type >= NONE_FLOW && type < N_FLOW);
	this->type = type;
}

FlowEnum FlowType::get() const {
	return type;
}

bool FlowType::operator==(FlowType type) const {
	return this->get() == type.get();
}

bool FlowType::operator!=(FlowType type) const {
	return this->get() != type.get();
}

std::string FlowType::toString() const {
	switch (type) {
		case DIR: return std::string("DIR");
		case ACU: return std::string("ACU");
		default: assert(0);
	}
}

std::string FlowType::code() const {
	assert(0);
}

} } // namespace map::detail
