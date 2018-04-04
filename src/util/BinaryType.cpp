/**
 * @file	BinaryType.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "BinaryType.hpp"
#include <cassert>


namespace map { namespace detail {

BinaryType::BinaryType()
	: type(NONE_BINARY)
{ }

BinaryType::BinaryType(BinaryEnum type) {
	assert(type >= NONE_BINARY && type < N_BINARY);
	this->type = type;
}

BinaryEnum BinaryType::get() const {
	return type;
}

bool BinaryType::operator==(BinaryType other) const {
	return this->get() == other.get();
}

bool BinaryType::operator!=(BinaryType other) const {
	return this->get() != other.get();
}

bool BinaryType::isOperator() const {
	return type < MARK_BINARY;
}

bool BinaryType::isFunction() const {
	return type > MARK_BINARY;
}

bool BinaryType::isRelational() const {
	return type >= EQ && type <= OR;
}

bool BinaryType::isBitwise() const {
	return type >= bAND && type <= SHR;
}

std::string BinaryType::toString() const {
	switch (type) {
		case NONE_BINARY: return std::string("NONE_BINARY");
		case ADD:  return std::string("ADD");
		case SUB:  return std::string("SUB");
		case MUL:  return std::string("MUL");
		case DIV:  return std::string("DIV");
		case MOD:  return std::string("MOD");
		case EQ:   return std::string("EQ");
		case NE:   return std::string("NE");
		case LT:   return std::string("LT");
		case GT:   return std::string("GT");
		case LE:   return std::string("LE");
		case GE:   return std::string("GE");
		case AND:  return std::string("AND");
		case OR:   return std::string("OR");
		case bAND: return std::string("bAND");
		case bOR:  return std::string("bOR");
		case bXOR: return std::string("bXOR");
		case SHL:  return std::string("SHL");
		case SHR:  return std::string("SHR");
		case MAX2: return std::string("MAX2");
		case MIN2: return std::string("MIN2");
		case ATAN2:return std::string("ATAN2");
		case POW:  return std::string("POW");
		case HYPOT:return std::string("HYPOT");
		case FMOD: return std::string("FMOD");
		default: assert(0);
	}
}

std::string BinaryType::code() const {
	switch (type) {
		case ADD:  return std::string("+");
		case SUB:  return std::string("-");
		case MUL:  return std::string("*");
		case DIV:  return std::string("/");
		case MOD:  return std::string("%");
		case EQ:   return std::string("==");
		case NE:   return std::string("!=");
		case LT:   return std::string("<");
		case GT:   return std::string(">");
		case LE:   return std::string("<=");
		case GE:   return std::string(">=");
		case AND:  return std::string("&&");
		case OR:   return std::string("||");
		case bAND: return std::string("&");
		case bOR:  return std::string("|");
		case bXOR: return std::string("^");
		case SHL:  return std::string("<<");
		case SHR:  return std::string(">>");
		case MAX2: return std::string("max");
		case MIN2: return std::string("min");
		case ATAN2:return std::string("atan2");
		case POW:  return std::string("pow");
		case HYPOT:return std::string("hypot");
		case FMOD: return std::string("fmod");
		default: assert(0);
	}
}

VariantType BinaryType::apply(VariantType lhs, VariantType rhs) const {
	switch (type) {
		case ADD  : return apply1< ADD  >(lhs,rhs);
		case SUB  : return apply1< SUB  >(lhs,rhs);
		case MUL  : return apply1< MUL  >(lhs,rhs);
		case DIV  : return apply1< DIV  >(lhs,rhs);
		case MOD  : return apply1< MOD  >(lhs,rhs);
		case EQ   : return apply1< EQ   >(lhs,rhs);
		case NE   : return apply1< NE   >(lhs,rhs);
		case LT   : return apply1< LT   >(lhs,rhs);
		case GT   : return apply1< GT   >(lhs,rhs);
		case LE   : return apply1< LE   >(lhs,rhs);
		case GE   : return apply1< GE   >(lhs,rhs);
		case AND  : return apply1< AND  >(lhs,rhs);
		case OR   : return apply1< OR   >(lhs,rhs);
		case bAND : return apply1< bAND >(lhs,rhs);
		case bOR  : return apply1< bOR  >(lhs,rhs);
		case bXOR : return apply1< bXOR >(lhs,rhs);
		case SHL  : return apply1< SHL  >(lhs,rhs);
		case SHR  : return apply1< SHR  >(lhs,rhs);
		case MAX2 : return apply1< MAX2 >(lhs,rhs);
		case MIN2 : return apply1< MIN2 >(lhs,rhs);
		case ATAN2: return apply1< ATAN2>(lhs,rhs);
		case POW  : return apply1< POW  >(lhs,rhs);
		case HYPOT: return apply1< HYPOT>(lhs,rhs);
		case FMOD : return apply1< FMOD >(lhs,rhs);
		default: assert(0);
	}
}

} } // namespace map::detail
