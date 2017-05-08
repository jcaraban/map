/**
 * @file	ReductionType.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "ReductionType.hpp"
#include <cassert>


namespace map { namespace detail {

ReductionEnum operator++(ReductionEnum& rte) {
   rte = static_cast<ReductionEnum>( static_cast<int>(rte)+1 );
   return rte;
}

ReductionType::ReductionType()
	: type(NONE_REDUCTION)
{ }

ReductionType::ReductionType(ReductionEnum type) {
	assert(type >= NONE_REDUCTION && type < N_REDUCTION);
	this->type = type;
}

ReductionEnum ReductionType::get() const {
	return type;
}

bool ReductionType::operator==(ReductionType type) const {
	return this->get() == type.get();
}

bool ReductionType::operator!=(ReductionType type) const {
	return this->get() != type.get();
}

bool ReductionType::isOperator() const {
	return type < MARK_REDUCTION;
}

bool ReductionType::isFunction() const {
	return type > MARK_REDUCTION;
}

std::string ReductionType::toString() const {
	switch (type) {
		case NONE_REDUCTION: return std::string("NONE_REDUCTION");
		case SUM : return std::string("SUM");
		case PROD: return std::string("PROD");
		case rAND: return std::string("rAND");
		case rOR : return std::string("rOR");
		case MAX : return std::string("MAX");
		case MIN : return std::string("MIN");
		default : assert(0);
	}
}

std::string ReductionType::code() const {
	switch (type) {
		case SUM : return std::string("+");
		case PROD: return std::string("*");
		case rAND: return std::string("&");
		case rOR : return std::string("|");
		case MAX : return std::string("max");
		case MIN : return std::string("min");
		default : assert(0);
	}
}

std::string ReductionType::neutralString(DataType dt) const {
	std::array<std::string,N_DATATYPE> min_str = {"/0","-INFINITY","-INFINITY","0","0","0","0","0","CHAR_MIN","SHRT_MIN","INT_MIN","LONG_MIN"};
	std::array<std::string,N_DATATYPE> max_str = {"/0","INFINITY","INFINITY","1","UCHAR_MAX","USHRT_MAX","UINT_MAX","ULONG_MAX","CHAR_MAX","SHRT_MAX","INT_MAX","LONG_MAX"};
	switch (type) {
		case SUM : return "0";
		case PROD: return "1";
		case rAND: return max_str[dt.get()];
		case rOR : return min_str[dt.get()];
		case MAX : return min_str[dt.get()];
		case MIN : return max_str[dt.get()];
		default: assert(0);
	}
}

VariantType ReductionType::neutral(DataType dt) const {
	switch (type) {
		case SUM : return neutral1<SUM >(dt);
		case PROD: return neutral1<PROD>(dt);
		case rAND: return neutral1<rAND>(dt);
		case rOR : return neutral1<rOR >(dt);
		case MAX : return neutral1<MAX >(dt);
		case MIN : return neutral1<MIN >(dt);
		default: assert(0);
	}
}

VariantType ReductionType::apply(VariantType lhs, VariantType rhs) const {
	assert(lhs.datatype() == rhs.datatype());
	switch (type) {
		case SUM : return apply1<SUM >(lhs,rhs);
		case PROD: return apply1<PROD>(lhs,rhs);
		case rAND: return apply1<rAND>(lhs,rhs);
		case rOR : return apply1<rOR >(lhs,rhs);
		case MAX : return apply1<MAX >(lhs,rhs);
		case MIN : return apply1<MIN >(lhs,rhs);
		default: assert(0);
	}
}

} } // namespace map::detail
