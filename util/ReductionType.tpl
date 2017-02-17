/**
 * @file	ReductionType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_UTIL_REDUCTIONTYPE_TPL_
#define MAP_UTIL_REDUCTIONTYPE_TPL_

#include <cmath>
#include <limits>
#include <cassert>


namespace map { namespace detail {

template <ReductionEnum R>
VariantType ReductionType::neutral1(DataType dt) const {
	switch (dt.get()) {
		case F32: return neutral2<R,F32>();
		case F64: return neutral2<R,F64>();
		case B8 : return neutral2<R,B8 >();
		case U8 : return neutral2<R,U8 >();
		case U16: return neutral2<R,U16>();
		case U32: return neutral2<R,U32>();
		case U64: return neutral2<R,U64>();
		case S8 : return neutral2<R,S8 >();
		case S16: return neutral2<R,S16>();
		case S32: return neutral2<R,S32>();
		case S64: return neutral2<R,S64>();
		default: assert(0);
	}
}

/*
 * NeutralOperator class, required for template-partial-specialization
 */
template <ReductionEnum R, DataTypeEnum T>
struct NeutralOperator {
	static_assert(true,"Not valid template parameter");
};

#define DEFINE_NEUTRAL_OPERATOR(R,expr) \
	template <DataTypeEnum T> \
	struct NeutralOperator<R,T> { \
		Ctype<T> operator()() { return expr; } \
	};

DEFINE_NEUTRAL_OPERATOR( SUM  , 0 )
DEFINE_NEUTRAL_OPERATOR( PROD , 1 )
DEFINE_NEUTRAL_OPERATOR( rAND , std::numeric_limits<Ctype<T>>::max() )
DEFINE_NEUTRAL_OPERATOR( rOR  , std::numeric_limits<Ctype<T>>::min() )
DEFINE_NEUTRAL_OPERATOR( MAX  , std::numeric_limits<Ctype<T>>::min() )
DEFINE_NEUTRAL_OPERATOR( MIN  , std::numeric_limits<Ctype<T>>::max() )
#undef DEFINE_NEUTRAL_OPERATOR

#define DEFINE_NEUTRAL_EXCEPTION(R,T,expr) \
	template <> \
	struct NeutralOperator<R,T> { \
		Ctype<T> operator()() { return expr; } \
	};

DEFINE_NEUTRAL_EXCEPTION( MAX , F32 , -std::numeric_limits<Ctype< F32 >>::infinity() )
DEFINE_NEUTRAL_EXCEPTION( MAX , F64 , -std::numeric_limits<Ctype< F64 >>::infinity() )
DEFINE_NEUTRAL_EXCEPTION( MIN , F32 , std::numeric_limits<Ctype< F32 >>::infinity() )
DEFINE_NEUTRAL_EXCEPTION( MIN , F64 , std::numeric_limits<Ctype< F64 >>::infinity() )
#undef DEFINE_NEUTRAL_EXCEPTION

template <ReductionEnum R, DataTypeEnum T>
VariantType ReductionType::neutral2() const {
	return VariantType( NeutralOperator<R,T>()() );
}

/********************************************************************************/

template <ReductionEnum R>
VariantType ReductionType::apply1(VariantType lhs, VariantType rhs) const {
	switch (lhs.datatype().get()) {
		case F32: return apply2<R,F32>(lhs,rhs);
		case F64: return apply2<R,F64>(lhs,rhs);
		case B8 : return apply2<R,B8 >(lhs,rhs);
		case U8 : return apply2<R,U8 >(lhs,rhs);
		case U16: return apply2<R,U16>(lhs,rhs);
		case U32: return apply2<R,U32>(lhs,rhs);
		case U64: return apply2<R,U64>(lhs,rhs);
		case S8 : return apply2<R,S8 >(lhs,rhs);
		case S16: return apply2<R,S16>(lhs,rhs);
		case S32: return apply2<R,S32>(lhs,rhs);
		case S64: return apply2<R,S64>(lhs,rhs);
		default: assert(0);
	}
}

/*
 * ReductionOperator class, required for template-partial-specialization
 */
template <ReductionEnum R, DataTypeEnum T>
struct ReductionOperator {
	static_assert(true,"Not valid template parameter");
};

#define DEFINE_REDUCTION_OPERATOR(R,expr) \
	template <DataTypeEnum T> \
	struct ReductionOperator<R,T> { \
		Ctype<T> operator()(Ctype<T> lhs, Ctype<T> rhs) { return expr; } \
	};

DEFINE_REDUCTION_OPERATOR( SUM  , lhs + rhs )
DEFINE_REDUCTION_OPERATOR( PROD , lhs * rhs )
DEFINE_REDUCTION_OPERATOR( rAND , lhs & rhs )
DEFINE_REDUCTION_OPERATOR( rOR  , lhs | rhs )
DEFINE_REDUCTION_OPERATOR( MAX  , std::max(lhs,rhs) )
DEFINE_REDUCTION_OPERATOR( MIN  , std::min(lhs,rhs) )
#undef DEFINE_REDUCTION_OPERATOR

#define DEFINE_REDUCTION_EXCEPTION(R,T,expr) \
	template <> \
	struct ReductionOperator<R,T> { \
		Ctype<T> operator()(Ctype<T> lhs, Ctype<T> rhs) { expr; } \
	};

DEFINE_REDUCTION_EXCEPTION( rAND , F32 , assert(0) )
DEFINE_REDUCTION_EXCEPTION( rOR  , F32 , assert(0) )
DEFINE_REDUCTION_EXCEPTION( rAND , F64 , assert(0) )
DEFINE_REDUCTION_EXCEPTION( rOR  , F64 , assert(0) )
#undef DEFINE_REDUCTION_EXCEPTION

template <ReductionEnum R, DataTypeEnum T>
VariantType ReductionType::apply2(VariantType lhs, VariantType rhs) const {
	return VariantType( ReductionOperator<R,T>()(lhs.get<T>(),rhs.get<T>()) );
}

} } // namespace detail, map

#endif
