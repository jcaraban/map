/**
 * @file	DiversityType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_UTIL_DIVERSITYTYPE_HPP_
#define MAP_UTIL_DIVERSITYTYPE_HPP_

#include "VariantType.hpp"
#include <string>
#include <vector>


namespace map { namespace detail {

// Enum

enum DiversityEnum : int {
	NONE_DIVERSITY, VARI, MAJO, MINO, MEAN, N_DIVERSITY
};

// Class

class DiversityType {
	DiversityEnum type;

  public:
	DiversityType();
	DiversityType(DiversityEnum type);
	DiversityEnum get() const;

	bool operator==(DiversityType type) const;
	bool operator!=(DiversityType type) const;

	std::string toString() const;
	VariantType apply(std::vector<VariantType> list) const;

	template <DiversityEnum D> static VariantType apply1(std::vector<VariantType> list);
	template <DiversityEnum D, DataTypeEnum T> static VariantType apply2(std::vector<VariantType> list);
};

static_assert( std::is_standard_layout< DiversityType >::value , "DiversityType must be C compatible");

} } // namespace map::detail

#endif

#include "DiversityType.tpl"
