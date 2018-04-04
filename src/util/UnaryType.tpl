/**
 * @file	UnaryType.tpl
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_UTIL_UNARYTYPE_TPL_
#define MAP_UTIL_UNARYTYPE_TPL_

#include <cmath>
#include <cassert>


namespace map { namespace detail {

template <UnaryEnum U>
VariantType UnaryType::apply1(VariantType var) const {
	switch (var.datatype().get()) {
		case F32: return apply2<U,F32>(var);
		case F64: return apply2<U,F64>(var);
		case B8 : return apply2<U,B8 >(var);
		case U8 : return apply2<U,U8 >(var);
		case U16: return apply2<U,U16>(var);
		case U32: return apply2<U,U32>(var);
		case U64: return apply2<U,U64>(var);
		case S8 : return apply2<U,S8 >(var);
		case S16: return apply2<U,S16>(var);
		case S32: return apply2<U,S32>(var);
		case S64: return apply2<U,S64>(var);
		default: assert(0);
	}
}

/*
 * Auxiliar Operator class, required for template-partial-specialization
 */
template <UnaryEnum U, DataTypeEnum T>
struct UnaryOperator {
	static_assert(true,"Not valid template parameter");
};

#define DEFINE_UNARY_OPERATOR(U,expr) \
	template <DataTypeEnum T> \
	struct UnaryOperator<U,T> { \
		Ctype<T> operator()(Ctype<T> var) { return expr; } \
	};

	DEFINE_UNARY_OPERATOR( POS   , +var )
	DEFINE_UNARY_OPERATOR( NEG   , -var )
	DEFINE_UNARY_OPERATOR( NOT   , !var )
	DEFINE_UNARY_OPERATOR( bNOT  , ~var )
	DEFINE_UNARY_OPERATOR( SIN   , std::sin(var) )
	DEFINE_UNARY_OPERATOR( COS   , std::cos(var) )
	DEFINE_UNARY_OPERATOR( TAN   , std::tan(var) )
	DEFINE_UNARY_OPERATOR( ASIN  , std::asin(var) )
	DEFINE_UNARY_OPERATOR( ACOS  , std::acos(var) )
	DEFINE_UNARY_OPERATOR( ATAN  , std::atan(var) )
	DEFINE_UNARY_OPERATOR( SINH  , std::sinh(var) )
	DEFINE_UNARY_OPERATOR( COSH  , std::cosh(var) )
	DEFINE_UNARY_OPERATOR( TANH  , std::tanh(var) )
	DEFINE_UNARY_OPERATOR( ASINH , std::asinh(var) )
	DEFINE_UNARY_OPERATOR( ACOSH , std::acosh(var) )
	DEFINE_UNARY_OPERATOR( ATANH , std::atanh(var) )
	DEFINE_UNARY_OPERATOR( EXP   , std::exp(var) )
	DEFINE_UNARY_OPERATOR( EXP2  , std::exp2(var) )
	DEFINE_UNARY_OPERATOR( EXP10 , std::pow(10,var) )
	DEFINE_UNARY_OPERATOR( LOG   , std::log(var) )
	DEFINE_UNARY_OPERATOR( LOG2  , std::log2(var) )
	DEFINE_UNARY_OPERATOR( LOG10 , std::log10(var) )
	DEFINE_UNARY_OPERATOR( SQRT  , std::sqrt(var) )
	DEFINE_UNARY_OPERATOR( CBRT  , std::cbrt(var) )
	DEFINE_UNARY_OPERATOR( ABS   , std::abs(var) )
	DEFINE_UNARY_OPERATOR( CEIL  , std::ceil(var) )
	DEFINE_UNARY_OPERATOR( FLOOR , std::floor(var) )
	DEFINE_UNARY_OPERATOR( TRUNC , std::trunc(var) )
	DEFINE_UNARY_OPERATOR( ROUND , std::round(var) )
#undef DEFINE_UNARY_OPERATOR

#define DEFINE_UNARY_EXCEPTION(U,T,expr) \
	template <> \
	struct UnaryOperator<U,T> { \
		Ctype<T> operator()(Ctype<T> var) { expr; } \
	};
	
	DEFINE_UNARY_EXCEPTION( bNOT , F32 , assert(0) )
	DEFINE_UNARY_EXCEPTION( bNOT , F64 , assert(0) )
	DEFINE_UNARY_EXCEPTION( bNOT , S8  , assert(0) )
	DEFINE_UNARY_EXCEPTION( bNOT , S16 , assert(0) )
	DEFINE_UNARY_EXCEPTION( bNOT , S32 , assert(0) )
	DEFINE_UNARY_EXCEPTION( bNOT , S64 , assert(0) )
#undef DEFINE_UNARY_EXCEPTION

template <UnaryEnum U, DataTypeEnum T>
VariantType UnaryType::apply2(VariantType var) const {
	return VariantType( UnaryOperator<U,T>()(var.get<T>()) );
}

} } // namespace detail, map

#endif
