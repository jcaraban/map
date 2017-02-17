/**
 * @file	DiversityType.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "DiversityType.hpp"


namespace map { namespace detail {

DiversityType::DiversityType()
	: type(NONE_DIVERSITY)
{ }

DiversityType::DiversityType(DiversityEnum type) {
	assert(type >= NONE_DIVERSITY && type < N_DIVERSITY);
	this->type = type;
}

DiversityEnum DiversityType::get() const {
	return type;
}

bool DiversityType::operator==(DiversityType type) const {
	return this->get() == type.get();
}

bool DiversityType::operator!=(DiversityType type) const {
	return this->get() == type.get();
}

std::string DiversityType::toString() const {
	switch (type) {
		case NONE_DIVERSITY: return std::string("NONE_DIVERSITY");
		case VARI: return std::string("VARI");
		case MAJO: return std::string("MAJO");
		case MINO: return std::string("MINO");
		case MEAN: return std::string("MEAN");
		default: assert(0);
	}
}

VariantType DiversityType::apply(std::vector<VariantType> list) const {
	assert(!list.empty());
	for (auto var : list)
		assert(var.datatype() == list[0].datatype());

	switch (type) {
		case VARI: return apply1< VARI >(list);
		case MAJO: return apply1< MAJO >(list);
		case MINO: return apply1< MINO >(list);
		case MEAN: return apply1< MEAN >(list);
		default: assert(0);
	}
}

} } // namespace map::detail
