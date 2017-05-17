/**
 * @file	BinaryType.hpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_UTIL_BINARYTYPE_HPP_
#define MAP_UTIL_BINARYTYPE_HPP_

#include "VariantType.hpp"
#include <string>


namespace map { namespace detail {

// Enum

enum BinaryEnum : int {
	NONE_BINARY, ADD, SUB, MUL, DIV, MOD, EQ, NE, LT, GT, LE, GE, AND, OR, bAND, bOR, bXOR, SHL, SHR,
	MARK_BINARY, MAX2, MIN2, ATAN2, POW, HYPOT, FMOD, N_BINARY
};

// Class

class BinaryType {
	BinaryEnum type;

  public:
	BinaryType();
	BinaryType(BinaryEnum type);
	BinaryEnum get() const;

	bool operator==(BinaryType other) const;
	bool operator!=(BinaryType other) const;

	bool isOperator() const;
	bool isFunction() const;
	bool isRelational() const;
	bool isBitwise() const;
	std::string toString() const;
	std::string code() const;
	VariantType apply(VariantType lhs, VariantType rhs) const;

  private:
	template <BinaryEnum B> VariantType apply1(VariantType lhs, VariantType rhs) const;
	template <BinaryEnum B, DataTypeEnum T> VariantType apply2(VariantType lhs, VariantType rhs) const;
	template <BinaryEnum B, DataTypeEnum T> VariantType apply3(VariantType lhs, VariantType rhs) const;
};

template <BinaryEnum U, DataTypeEnum T> struct BinaryOperator;

static_assert( std::is_standard_layout< BinaryType >::value , "BinaryType must be C compatible");

} } // namespace map::detail

#endif

#include "BinaryType.tpl"
