/**
 * @file	VariantType.tpl
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_UTIL_VARIANT_TPL_
#define MAP_UTIL_VARIANT_TPL_

#include <cassert>


namespace map { namespace detail {

template <DataTypeEnum T> Ctype<T>& VariantType::get() { static_assert(true,"Not valid template parameter"); }
template <DataTypeEnum T> const Ctype<T>& VariantType::get() const { static_assert(true,"Not valid template parameter"); }
#define DEFINE_VARIANT_GET(T,tX) \
	template <> inline Ctype<T>& VariantType::get<T>() { assert(type==T); return var.tX; } \
	template <> inline const Ctype<T>& VariantType::get<T>() const { assert(type==T); return var.tX; }
DEFINE_VARIANT_GET( F32 , f32 )
DEFINE_VARIANT_GET( F64 , f64 )
DEFINE_VARIANT_GET( B8  , b8  )
DEFINE_VARIANT_GET( U8  , u8  )
DEFINE_VARIANT_GET( U16 , u16 )
DEFINE_VARIANT_GET( U32 , u32 )
DEFINE_VARIANT_GET( U64 , u64 )
DEFINE_VARIANT_GET( S8  , s8  )
DEFINE_VARIANT_GET( S16 , s16 )
DEFINE_VARIANT_GET( S32 , s32 )
DEFINE_VARIANT_GET( S64 , s64 )
#undef DEFINE_VARIANT_GET

template <typename T> VariantType& VariantType::set(T val, DataType dt) { static_assert(true,"Not valid template parameter"); }
#define DEFINE_VARIANT_SET(T) \
	template<> inline \
	VariantType& VariantType::set<Ctype<T>>(Ctype<T> val, DataType dt) { \
		type = dt; \
		switch (type.get()) { \
			case F32 : get<F32>() = val; break; \
			case F64 : get<F64>() = val; break; \
			case B8  : get<B8 >() = val; break; \
			case U8  : get<U8 >() = val; break; \
			case U16 : get<U16>() = val; break; \
			case U32 : get<U32>() = val; break; \
			case U64 : get<U64>() = val; break; \
			case S8  : get<S8 >() = val; break; \
			case S16 : get<S16>() = val; break; \
			case S32 : get<S32>() = val; break; \
			case S64 : get<S64>() = val; break; \
			default  : assert(0); \
		} \
		return *this; \
	}
DEFINE_VARIANT_SET( F32 )
DEFINE_VARIANT_SET( F64 )
DEFINE_VARIANT_SET( B8  )
DEFINE_VARIANT_SET( U8  )
DEFINE_VARIANT_SET( U16 )
DEFINE_VARIANT_SET( U32 )
DEFINE_VARIANT_SET( U64 )
DEFINE_VARIANT_SET( S8  )
DEFINE_VARIANT_SET( S16 )
DEFINE_VARIANT_SET( S32 )
DEFINE_VARIANT_SET( S64 )
#undef DEFINE_VARIANT_SET

} } // namespace map::detail

#endif
