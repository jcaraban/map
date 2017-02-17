/**
 * @file	promote.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Promotion of types defines to what type is promoted a pair of types
 */

#ifndef MAP_UTIL_PROMOTE_HPP_
#define MAP_UTIL_PROMOTE_HPP_

#include "DataType.hpp"
#include <cassert>


namespace map { namespace detail {

template <DataTypeEnum T1, DataTypeEnum T2> struct Promote { static_assert(true,"Invalid template paremeters"); };

// @ a partial template specialization would cover the promoting of sames
//template <DataTypeEnum T> struct Promote<T,T> { static const DataTypeEnum value = T; };

#define DEFINE_PROMOTE(T1,T2,T3) template <> struct Promote<T1,T2> { static const DataTypeEnum value = T3; };
DEFINE_PROMOTE( F32 , F32 , F32 )
DEFINE_PROMOTE( F32 , F64 , F64 )
DEFINE_PROMOTE( F32 , B8  , F32 )
DEFINE_PROMOTE( F32 , U8  , F32 )
DEFINE_PROMOTE( F32 , U16 , F32 )
DEFINE_PROMOTE( F32 , U32 , F32 )
DEFINE_PROMOTE( F32 , U64 , F32 )
DEFINE_PROMOTE( F32 , S8  , F32 )
DEFINE_PROMOTE( F32 , S16 , F32 )
DEFINE_PROMOTE( F32 , S32 , F32 )
DEFINE_PROMOTE( F32 , S64 , F32 )

DEFINE_PROMOTE( F64 , F32 , F64 )
DEFINE_PROMOTE( F64 , F64 , F64 )
DEFINE_PROMOTE( F64 , B8  , F64 )
DEFINE_PROMOTE( F64 , U8  , F64 )
DEFINE_PROMOTE( F64 , U16 , F64 )
DEFINE_PROMOTE( F64 , U32 , F64 )
DEFINE_PROMOTE( F64 , U64 , F64 )
DEFINE_PROMOTE( F64 , S8  , F64 )
DEFINE_PROMOTE( F64 , S16 , F64 )
DEFINE_PROMOTE( F64 , S32 , F64 )
DEFINE_PROMOTE( F64 , S64 , F64 ) 

DEFINE_PROMOTE( B8  , F32 , F32 )
DEFINE_PROMOTE( B8  , F64 , F64 )
DEFINE_PROMOTE( B8  , B8  , B8  )
DEFINE_PROMOTE( B8  , U8  , U8 )
DEFINE_PROMOTE( B8  , U16 , U16 )
DEFINE_PROMOTE( B8  , U32 , U32 )
DEFINE_PROMOTE( B8  , U64 , U64 )
DEFINE_PROMOTE( B8  , S8  , S8 )
DEFINE_PROMOTE( B8  , S16 , S16 )
DEFINE_PROMOTE( B8  , S32 , S32 )
DEFINE_PROMOTE( B8  , S64 , S64 )

DEFINE_PROMOTE( U8  , F32 , F32 )
DEFINE_PROMOTE( U8  , F64 , F64 )
DEFINE_PROMOTE( U8  , B8  , U8 )
DEFINE_PROMOTE( U8  , U8  , U8 )
DEFINE_PROMOTE( U8  , U16 , U16 )
DEFINE_PROMOTE( U8  , U32 , U32 )
DEFINE_PROMOTE( U8  , U64 , U64 )
DEFINE_PROMOTE( U8  , S8  , S8 )
DEFINE_PROMOTE( U8  , S16 , S16 )
DEFINE_PROMOTE( U8  , S32 , S32 )
DEFINE_PROMOTE( U8  , S64 , S64 )

DEFINE_PROMOTE( U16 , F32 , F32 )
DEFINE_PROMOTE( U16 , F64 , F64 )
DEFINE_PROMOTE( U16 , B8  , U16 )
DEFINE_PROMOTE( U16 , U8  , U16 )
DEFINE_PROMOTE( U16 , U16 , U16 )
DEFINE_PROMOTE( U16 , U32 , U32 )
DEFINE_PROMOTE( U16 , U64 , U64 )
DEFINE_PROMOTE( U16 , S8  , U16 )
DEFINE_PROMOTE( U16 , S16 , S16 )
DEFINE_PROMOTE( U16 , S32 , S32 )
DEFINE_PROMOTE( U16 , S64 , S64 )

DEFINE_PROMOTE( U32 , F32 , F32 )
DEFINE_PROMOTE( U32 , F64 , F64 )
DEFINE_PROMOTE( U32 , B8  , U32 )
DEFINE_PROMOTE( U32 , U8  , U32 )
DEFINE_PROMOTE( U32 , U16 , U32 )
DEFINE_PROMOTE( U32 , U32 , U32 )
DEFINE_PROMOTE( U32 , U64 , U64 )
DEFINE_PROMOTE( U32 , S8  , S32 )
DEFINE_PROMOTE( U32 , S16 , S32 )
DEFINE_PROMOTE( U32 , S32 , S32 )
DEFINE_PROMOTE( U32 , S64 , S64 )

DEFINE_PROMOTE( U64 , F32 , F32 )
DEFINE_PROMOTE( U64 , F64 , F64 )
DEFINE_PROMOTE( U64 , B8  , B8  )
DEFINE_PROMOTE( U64 , U8  , U64 )
DEFINE_PROMOTE( U64 , U16 , U64 )
DEFINE_PROMOTE( U64 , U32 , U64 )
DEFINE_PROMOTE( U64 , U64 , U64 )
DEFINE_PROMOTE( U64 , S8  , S64 )
DEFINE_PROMOTE( U64 , S16 , S64 )
DEFINE_PROMOTE( U64 , S32 , S64 )
DEFINE_PROMOTE( U64 , S64 , S64 )

DEFINE_PROMOTE( S8 , F32 , F32 )
DEFINE_PROMOTE( S8 , F64 , F64 )
DEFINE_PROMOTE( S8 , B8  , S8 )
DEFINE_PROMOTE( S8 , U8  , S8 )
DEFINE_PROMOTE( S8 , U16 , S16 )
DEFINE_PROMOTE( S8 , U32 , S32 )
DEFINE_PROMOTE( S8 , U64 , S64 )
DEFINE_PROMOTE( S8 , S8  , S8 )
DEFINE_PROMOTE( S8 , S16 , S16 )
DEFINE_PROMOTE( S8 , S32 , S32 )
DEFINE_PROMOTE( S8 , S64 , S64 )

DEFINE_PROMOTE( S16  , F32 , F32 )
DEFINE_PROMOTE( S16  , F64 , F64 )
DEFINE_PROMOTE( S16  , B8  , S16 )
DEFINE_PROMOTE( S16  , U8  , S16 )
DEFINE_PROMOTE( S16  , U16 , S16 )
DEFINE_PROMOTE( S16  , U32 , S32 )
DEFINE_PROMOTE( S16  , U64 , S64 )
DEFINE_PROMOTE( S16  , S8  , S16 )
DEFINE_PROMOTE( S16  , S16 , S16 )
DEFINE_PROMOTE( S16  , S32 , S32 )
DEFINE_PROMOTE( S16  , S64 , S64 )

DEFINE_PROMOTE( S32  , F32 , F32 )
DEFINE_PROMOTE( S32  , F64 , F64 )
DEFINE_PROMOTE( S32  , B8  , S32 )
DEFINE_PROMOTE( S32  , U8  , S32 )
DEFINE_PROMOTE( S32  , U16 , S32 )
DEFINE_PROMOTE( S32  , U32 , S32 )
DEFINE_PROMOTE( S32  , U64 , S64 )
DEFINE_PROMOTE( S32  , S8  , S32 )
DEFINE_PROMOTE( S32  , S16 , S32 )
DEFINE_PROMOTE( S32  , S32 , S32 )
DEFINE_PROMOTE( S32  , S64 , S64 )

DEFINE_PROMOTE( S64  , F32 , F32 )
DEFINE_PROMOTE( S64  , F64 , F64 )
DEFINE_PROMOTE( S64  , B8  , S64 )
DEFINE_PROMOTE( S64  , U8  , S64 )
DEFINE_PROMOTE( S64  , U16 , S64 )
DEFINE_PROMOTE( S64  , U32 , S64 )
DEFINE_PROMOTE( S64  , U64 , S64 )
DEFINE_PROMOTE( S64  , S8  , S64 )
DEFINE_PROMOTE( S64  , S16 , S64 )
DEFINE_PROMOTE( S64  , S32 , S64 )
DEFINE_PROMOTE( S64  , S64 , S64 )
#undef DEFINE_PROMOTE

// Runtime version

inline DataType promote(DataType dt1, DataType dt2) {
	switch (dt1.get()) {
		case F32: switch (dt2.get())
		{
			case F32 : return Promote<F32,F32>::value;
			case F64 : return Promote<F32,F64>::value;
			case B8  : return Promote<F32,B8 >::value;
			case U8  : return Promote<F32,U8 >::value;
			case U16 : return Promote<F32,U16>::value;
			case U32 : return Promote<F32,U32>::value;
			case U64 : return Promote<F32,U64>::value;
			case S8  : return Promote<F32,S8 >::value;
			case S16 : return Promote<F32,S16>::value;
			case S32 : return Promote<F32,S32>::value;
			case S64 : return Promote<F32,S64>::value;
			default  : assert(0);
		}
		case F64: switch (dt2.get())
		{
			case F32 : return Promote<F64,F32>::value;
			case F64 : return Promote<F64,F64>::value;
			case B8  : return Promote<F64,B8 >::value;
			case U8  : return Promote<F64,U8 >::value;
			case U16 : return Promote<F64,U16>::value;
			case U32 : return Promote<F64,U32>::value;
			case U64 : return Promote<F64,U64>::value;
			case S8  : return Promote<F64,S8 >::value;
			case S16 : return Promote<F64,S16>::value;
			case S32 : return Promote<F64,S32>::value;
			case S64 : return Promote<F64,S64>::value;
			default  : assert(0);
		}
		case B8: switch (dt2.get())
		{
			case F32 : return Promote<B8 ,F32>::value;
			case F64 : return Promote<B8 ,F64>::value;
			case B8  : return Promote<B8 ,B8 >::value;
			case U8  : return Promote<B8 ,U8 >::value;
			case U16 : return Promote<B8 ,U16>::value;
			case U32 : return Promote<B8 ,U32>::value;
			case U64 : return Promote<B8 ,U64>::value;
			case S8  : return Promote<B8 ,S8 >::value;
			case S16 : return Promote<B8 ,S16>::value;
			case S32 : return Promote<B8 ,S32>::value;
			case S64 : return Promote<B8 ,S64>::value;
			default  : assert(0);
		}
		case U8: switch (dt2.get())
		{
			case F32 : return Promote<U8 ,F32>::value;
			case F64 : return Promote<U8 ,F64>::value;
			case B8  : return Promote<U8 ,B8 >::value;
			case U8  : return Promote<U8 ,U8 >::value;
			case U16 : return Promote<U8 ,U16>::value;
			case U32 : return Promote<U8 ,U32>::value;
			case U64 : return Promote<U8 ,U64>::value;
			case S8  : return Promote<U8 ,S8 >::value;
			case S16 : return Promote<U8 ,S16>::value;
			case S32 : return Promote<U8 ,S32>::value;
			case S64 : return Promote<U8 ,S64>::value;
			default  : assert(0);
		}
		case U16: switch (dt2.get())
		{
			case F32 : return Promote<U16,F32>::value;
			case F64 : return Promote<U16,F64>::value;
			case B8  : return Promote<U16,B8 >::value;
			case U8  : return Promote<U16,U8 >::value;
			case U16 : return Promote<U16,U16>::value;
			case U32 : return Promote<U16,U32>::value;
			case U64 : return Promote<U16,U64>::value;
			case S8  : return Promote<U16,S8 >::value;
			case S16 : return Promote<U16,S16>::value;
			case S32 : return Promote<U16,S32>::value;
			case S64 : return Promote<U16,S64>::value;
			default  : assert(0);
		}
		case U32: switch (dt2.get())
		{
			case F32 : return Promote<U32,F32>::value;
			case F64 : return Promote<U32,F64>::value;
			case B8  : return Promote<U32,B8 >::value;
			case U8  : return Promote<U32,U8 >::value;
			case U16 : return Promote<U32,U16>::value;
			case U32 : return Promote<U32,U32>::value;
			case U64 : return Promote<U32,U64>::value;
			case S8  : return Promote<U32,S8 >::value;
			case S16 : return Promote<U32,S16>::value;
			case S32 : return Promote<U32,S32>::value;
			case S64 : return Promote<U32,S64>::value;
			default  : assert(0);
		}
		case U64: switch (dt2.get())
		{
			case F32 : return Promote<U64,F32>::value;
			case F64 : return Promote<U64,F64>::value;
			case B8  : return Promote<U64,B8 >::value;
			case U8  : return Promote<U64,U8 >::value;
			case U16 : return Promote<U64,U16>::value;
			case U32 : return Promote<U64,U32>::value;
			case U64 : return Promote<U64,U64>::value;
			case S8  : return Promote<U64,S8 >::value;
			case S16 : return Promote<U64,S16>::value;
			case S32 : return Promote<U64,S32>::value;
			case S64 : return Promote<U64,S64>::value;
			default  : assert(0);
		}
		case S8 : switch (dt2.get())
		{
			case F32 : return Promote<S8 ,F32>::value;
			case F64 : return Promote<S8 ,F64>::value;
			case B8  : return Promote<S8 ,B8 >::value;
			case U8  : return Promote<S8 ,U8 >::value;
			case U16 : return Promote<S8 ,U16>::value;
			case U32 : return Promote<S8 ,U32>::value;
			case U64 : return Promote<S8 ,U64>::value;
			case S8  : return Promote<S8 ,S8 >::value;
			case S16 : return Promote<S8 ,S16>::value;
			case S32 : return Promote<S8 ,S32>::value;
			case S64 : return Promote<S8 ,S64>::value;
			default  : assert(0);
		}
		case S16: switch (dt2.get())
		{
			case F32 : return Promote<S16,F32>::value;
			case F64 : return Promote<S16,F64>::value;
			case B8  : return Promote<S16,B8 >::value;
			case U8  : return Promote<S16,U8 >::value;
			case U16 : return Promote<S16,U16>::value;
			case U32 : return Promote<S16,U32>::value;
			case U64 : return Promote<S16,U64>::value;
			case S8  : return Promote<S16,S8 >::value;
			case S16 : return Promote<S16,S16>::value;
			case S32 : return Promote<S16,S32>::value;
			case S64 : return Promote<S16,S64>::value;
			default  : assert(0);
		}
		case S32: switch (dt2.get())
		{
			case F32 : return Promote<S32,F32>::value;
			case F64 : return Promote<S32,F64>::value;
			case B8  : return Promote<S32,B8 >::value;
			case U8  : return Promote<S32,U8 >::value;
			case U16 : return Promote<S32,U16>::value;
			case U32 : return Promote<S32,U32>::value;
			case U64 : return Promote<S32,U64>::value;
			case S8  : return Promote<S32,S8 >::value;
			case S16 : return Promote<S32,S16>::value;
			case S32 : return Promote<S32,S32>::value;
			case S64 : return Promote<S32,S64>::value;
			default  : assert(0);
		}
		case S64: switch (dt2.get())
		{
			case F32 : return Promote<S64,F32>::value;
			case F64 : return Promote<S64,F64>::value;
			case B8  : return Promote<S64,B8 >::value;
			case U8  : return Promote<S64,U8 >::value;
			case U16 : return Promote<S64,U16>::value;
			case U32 : return Promote<S64,U32>::value;
			case U64 : return Promote<S64,U64>::value;
			case S8  : return Promote<S64,S8 >::value;
			case S16 : return Promote<S64,S16>::value;
			case S32 : return Promote<S64,S32>::value;
			case S64 : return Promote<S64,S64>::value;
			default  : assert(0);
		}
		default: assert(0);
	}
}

} } // namespace map::detail

#endif
