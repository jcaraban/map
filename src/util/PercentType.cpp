/**
 * @file	PercentType.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "PercentType.hpp"
#include <cassert>


namespace map { namespace detail {

PercentType::PercentType()
	: type(NONE_PERCENT)
{ }

PercentType::PercentType(PercentEnum type) {
	assert(type >= NONE_PERCENT && type < N_PERCENT);
	this->type = type;
}

PercentEnum PercentType::get() const {
	return type;
}

bool PercentType::operator==(PercentType type) const {
	return this->get() == type.get();
}

bool PercentType::operator!=(PercentType type) const {
	return this->get() != type.get();
}

std::string PercentType::toString() const {
	switch (type) {
		case AGE: return std::string("AGE");
		case ILE: return std::string("ILE");
		default: assert(0);
	}
}

std::string PercentType::code() const {
	switch (type) {
		case AGE: return std::string("==");
		case ILE: return std::string(">");
		default: assert(0);
	}
}

} } // namespace map::detail
