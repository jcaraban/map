/**
 * @file	UnaryType.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "UnaryType.hpp"
#include <cassert>


namespace map { namespace detail {

UnaryType::UnaryType()
	: type(NONE_UNARY)
{ }

UnaryType::UnaryType(UnaryEnum type)
	: type(type)
{
	assert(type >= NONE_UNARY && type < N_UNARY);
}

UnaryEnum UnaryType::get() const {
	return type;
}

bool UnaryType::operator==(UnaryType other) const {
	return this->get() == other.get();
}

bool UnaryType::operator!=(UnaryType other) const {
	return this->get() != other.get();
}

bool UnaryType::isOperator() const {
	return type < MARK_UNARY;
}

bool UnaryType::isFunction() const {
	return type > MARK_UNARY;
}

bool UnaryType::isRelational() const {
	return type == NOT;
}

bool UnaryType::isBitwise() const {
	return type == bNOT;
}

std::string UnaryType::toString() const {
	switch (type) {
		case NONE_UNARY: return std::string("NONE_UNARY");
		case POS  : return std::string("POS");
		case NEG  : return std::string("NEG");
		case NOT  : return std::string("NOT");
		case bNOT : return std::string("bNOT");
		case MARK_UNARY: return std::string("MARK_UNARY");
		case SIN  : return std::string("SIN");
		case COS  : return std::string("COS");
		case TAN  : return std::string("TAN");
		case ASIN : return std::string("ASIN");
		case ACOS : return std::string("ACOS");
		case ATAN : return std::string("ATAN");
		case SINH : return std::string("SINH");
		case COSH : return std::string("COSH");
		case TANH : return std::string("TANH");
		case ASINH: return std::string("ASINH");
		case ACOSH: return std::string("ACOSH");
		case ATANH: return std::string("ATANH");
		case EXP  : return std::string("EXP");
		case EXP2 : return std::string("EXP2");
		case EXP10: return std::string("EXP10");
		case LOG  : return std::string("LOG");
		case LOG2 : return std::string("LOG2");
		case LOG10: return std::string("LOG10");
		case SQRT : return std::string("SQRT");
		case CBRT : return std::string("CBRT");
		case ABS  : return std::string("ABS");
		case CEIL : return std::string("CEIL");
		case FLOOR: return std::string("FLOOR");
		case TRUNC: return std::string("TRUNC");
		case ROUND: return std::string("ROUND");
		default: assert(0);
	}
}

std::string UnaryType::code() const {
	switch (type) {
		case POS  : return std::string("+");
		case NEG  : return std::string("-");
		case NOT  : return std::string("!");
		case bNOT : return std::string("~");
		case SIN  : return std::string("sin");
		case COS  : return std::string("cos");
		case TAN  : return std::string("tan");
		case ASIN : return std::string("asin");
		case ACOS : return std::string("acos");
		case ATAN : return std::string("atan");
		case SINH : return std::string("sinh");
		case COSH : return std::string("cosh");
		case TANH : return std::string("tanh");
		case ASINH: return std::string("asinh");
		case ACOSH: return std::string("acosh");
		case ATANH: return std::string("atanh");
		case EXP  : return std::string("exp");
		case EXP2 : return std::string("exp2");
		case EXP10: return std::string("exp10");
		case LOG  : return std::string("log");
		case LOG2 : return std::string("log2");
		case LOG10: return std::string("log10");
		case SQRT : return std::string("sqrt");
		case CBRT : return std::string("cbrt");
		case ABS  : return std::string("fabs");
		case CEIL : return std::string("ceil");
		case FLOOR: return std::string("floor");
		case TRUNC: return std::string("trunc");
		case ROUND: return std::string("round");
		default: assert(0);
	}
}

VariantType UnaryType::apply(VariantType var) const {
	switch (type) {
		case POS  : return apply1<POS  >(var);
		case NEG  : return apply1<NEG  >(var);
		case NOT  : return apply1<NOT  >(var);
		case bNOT : return apply1<bNOT >(var);
		case SIN  : return apply1<SIN  >(var);
		case COS  : return apply1<COS  >(var);
		case TAN  : return apply1<TAN  >(var);
		case ASIN : return apply1<ASIN >(var);
		case ACOS : return apply1<ACOS >(var);
		case ATAN : return apply1<ATAN >(var);
		case SINH : return apply1<SINH >(var);
		case COSH : return apply1<COSH >(var);
		case TANH : return apply1<TANH >(var);
		case ASINH: return apply1<ASINH>(var);
		case ACOSH: return apply1<ACOSH>(var);
		case ATANH: return apply1<ATANH>(var);
		case EXP  : return apply1<EXP  >(var);
		case EXP2 : return apply1<EXP2 >(var);
		case EXP10: return apply1<EXP10>(var);
		case LOG  : return apply1<LOG  >(var);
		case LOG2 : return apply1<LOG2 >(var);
		case LOG10: return apply1<LOG10>(var);
		case SQRT : return apply1<SQRT >(var);
		case CBRT : return apply1<CBRT >(var);
		case ABS  : return apply1<ABS  >(var);
		case CEIL : return apply1<CEIL >(var);
		case FLOOR: return apply1<FLOOR>(var);
		case TRUNC: return apply1<TRUNC>(var);
		case ROUND: return apply1<ROUND>(var);
		default: assert(0);
	}
}

} } // namespace map::detail
