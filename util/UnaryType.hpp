/**
 * @file	UnaryType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_UTIL_UNARYTYPE_HPP_
#define MAP_UTIL_UNARYTYPE_HPP_

#include "VariantType.hpp"
#include <string>


namespace map { namespace detail {

// Enum

enum UnaryEnum : int {
	NONE_UNARY, POS, NEG, NOT, bNOT, MARK_UNARY,
	SIN, COS, TAN, ASIN, ACOS, ATAN, SINH, COSH, TANH, ASINH, ACOSH, ATANH,
	EXP, EXP2, EXP10, LOG, LOG2, LOG10,
	SQRT, CBRT, ABS,
	CEIL, FLOOR, TRUNC, ROUND, N_UNARY
};

// Class

class UnaryType {
	UnaryEnum type;

  public:
	UnaryType();
	UnaryType(UnaryEnum type);
	UnaryEnum get() const;

	bool operator==(UnaryType other) const;
	bool operator!=(UnaryType other) const;

	bool isOperator() const;
	bool isFunction() const;
	bool isRelational() const;
	bool isBitwise() const;
	std::string toString() const;
	std::string code() const;
	VariantType apply(VariantType var) const;

  private:
	template <UnaryEnum U> VariantType apply1(VariantType var) const;
	template <UnaryEnum U, DataTypeEnum T> VariantType apply2(VariantType var) const;
};

template <UnaryEnum U, DataTypeEnum T> struct UnaryOperator;

static_assert( std::is_standard_layout< UnaryType >::value , "UnaryType must be C compatible");

} } // namespace map::detail

#endif

#include "UnaryType.tpl"
