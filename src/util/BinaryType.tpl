/**
 * @file	BinaryType.tpl
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: the promotion would look better in apply3. And apply3 is necessary anyway
 *       however that would generate x^2 times more code paths
 */

#ifndef MAP_UTIL_BINARYTYPE_TPL_
#define MAP_UTIL_BINARYTYPE_TPL_

#include "promote.hpp"
#include <cmath>
#include <cassert>

namespace map { namespace detail {

template <BinaryEnum B>
VariantType BinaryType::apply1(VariantType lhs, VariantType rhs) const {
	switch (lhs.datatype().get()) {
		case F32: return apply2<B,F32>(lhs,rhs);
		case F64: return apply2<B,F64>(lhs,rhs);
		case B8 : return apply2<B,B8 >(lhs,rhs);
		case U8 : return apply2<B,U8 >(lhs,rhs);
		case U16: return apply2<B,U16>(lhs,rhs);
		case U32: return apply2<B,U32>(lhs,rhs);
		case U64: return apply2<B,U64>(lhs,rhs);
		case S8 : return apply2<B,S8 >(lhs,rhs);
		case S16: return apply2<B,S16>(lhs,rhs);
		case S32: return apply2<B,S32>(lhs,rhs);
		case S64: return apply2<B,S64>(lhs,rhs);
		default: assert(0);
	}
}

template <BinaryEnum B, DataTypeEnum T>
VariantType BinaryType::apply2(VariantType lhs, VariantType rhs) const {
	//#define PRO(T) Promote<T1,T>::value
	switch (rhs.datatype().get()) {
		case F32: return apply3<B,Promote<T,F32>::value>(lhs,rhs);
		case F64: return apply3<B,Promote<T,F64>::value>(lhs,rhs);
		case B8 : return apply3<B,Promote<T,B8 >::value>(lhs,rhs);
		case U8 : return apply3<B,Promote<T,U8 >::value>(lhs,rhs);
		case U16: return apply3<B,Promote<T,U16>::value>(lhs,rhs);
		case U32: return apply3<B,Promote<T,U32>::value>(lhs,rhs);
		case U64: return apply3<B,Promote<T,U64>::value>(lhs,rhs);
		case S8 : return apply3<B,Promote<T,S8 >::value>(lhs,rhs);
		case S16: return apply3<B,Promote<T,S16>::value>(lhs,rhs);
		case S32: return apply3<B,Promote<T,S32>::value>(lhs,rhs);
		case S64: return apply3<B,Promote<T,S64>::value>(lhs,rhs);
		default: assert(0);
	}
	//#undef PRO
}

/*
 * Auxiliar Operator class, required for template-partial-specialization
 */
template <BinaryEnum B, DataTypeEnum T>
struct BinaryOperator {
	static_assert(true,"Not valid template parameter");
};

#define DEFINE_BINARY_OPERATOR(B,expr) \
	template <DataTypeEnum T> \
	struct BinaryOperator<B,T> { \
		Ctype<T> operator()(Ctype<T> lhs, Ctype<T> rhs) { return expr; } \
	};

#define DEFINE_BINARY_RELATION(B,expr) \
	template <DataTypeEnum T> \
	struct BinaryOperator<B,T> { \
		/*@*/unsigned char operator()(Ctype<T> lhs, Ctype<T> rhs) { return expr; } \
	};

DEFINE_BINARY_OPERATOR( ADD , lhs + rhs )
DEFINE_BINARY_OPERATOR( SUB , lhs - rhs )
DEFINE_BINARY_OPERATOR( MUL , lhs * rhs )
DEFINE_BINARY_OPERATOR( DIV , lhs / rhs )
DEFINE_BINARY_OPERATOR( MOD , lhs % rhs )
DEFINE_BINARY_RELATION( EQ , lhs == rhs )
DEFINE_BINARY_RELATION( NE , lhs != rhs )
DEFINE_BINARY_RELATION( LT , lhs < rhs )
DEFINE_BINARY_RELATION( GT , lhs > rhs )
DEFINE_BINARY_RELATION( LE , lhs <= rhs )
DEFINE_BINARY_RELATION( GE , lhs >= rhs )
DEFINE_BINARY_RELATION( AND , lhs && rhs )
DEFINE_BINARY_RELATION( OR  , lhs || rhs )
DEFINE_BINARY_OPERATOR( bAND , lhs & rhs )
DEFINE_BINARY_OPERATOR( bOR  , lhs | rhs )
DEFINE_BINARY_OPERATOR( bXOR , lhs ^ rhs )
DEFINE_BINARY_OPERATOR( SHL  , lhs << rhs )
DEFINE_BINARY_OPERATOR( SHR  , lhs >> rhs )
DEFINE_BINARY_OPERATOR( MAX2  , std::max(lhs,rhs) )
DEFINE_BINARY_OPERATOR( MIN2  , std::min(lhs,rhs) )
DEFINE_BINARY_OPERATOR( ATAN2 , std::atan2(lhs,rhs) )
DEFINE_BINARY_OPERATOR( POW   , std::pow(lhs,rhs) )
DEFINE_BINARY_OPERATOR( HYPOT , std::hypot(lhs,rhs) )
DEFINE_BINARY_OPERATOR( FMOD  , std::fmod(lhs,rhs) )
#undef DEFINE_BINARY_OPERATOR
#undef DEFINE_BINARY_RELATION

#define DEFINE_BINARY_EXCEPTION(B,T,expr) \
	template <> \
	struct BinaryOperator<B,T> { \
		Ctype<T> operator()(Ctype<T> lhs, Ctype<T> rhs) { expr; } \
	};

DEFINE_BINARY_EXCEPTION( MOD , F32 , assert(0) )
DEFINE_BINARY_EXCEPTION( MOD , F64 , assert(0) )
DEFINE_BINARY_EXCEPTION( bAND , F32 , assert(0) )
DEFINE_BINARY_EXCEPTION( bAND , F64 , assert(0) )
DEFINE_BINARY_EXCEPTION( bOR , F32 , assert(0) )
DEFINE_BINARY_EXCEPTION( bOR , F64 , assert(0) )
DEFINE_BINARY_EXCEPTION( bXOR , F32 , assert(0) )
DEFINE_BINARY_EXCEPTION( bXOR , F64 , assert(0) )
DEFINE_BINARY_EXCEPTION( SHL , F32 , assert(0) )
DEFINE_BINARY_EXCEPTION( SHL , F64 , assert(0) )
DEFINE_BINARY_EXCEPTION( SHR , F32 , assert(0) )
DEFINE_BINARY_EXCEPTION( SHR , F64 , assert(0) )
#undef DEFINE_BINARY_EXCEPTION

template <BinaryEnum B, DataTypeEnum T>
VariantType BinaryType::apply3(VariantType lhs, VariantType rhs) const {
	Ctype<T> lhst = lhs.convert(T).get<T>();
	Ctype<T> rhst = rhs.convert(T).get<T>();
	return VariantType( BinaryOperator<B,T>()(lhst,rhst) );
}

} } // namespace map::detail

#endif
