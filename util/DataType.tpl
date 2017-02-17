/**
 * @file	DataType.tpl
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_UTIL_DATATYPE_TPL_
#define MAP_UTIL_DATATYPE_TPL_


namespace map { namespace detail {

template <DataTypeEnum TYPE> struct DataType2Ctype { static_assert(true,"Invalid DataType template parameter"); };
template <> struct DataType2Ctype< F32 > { typedef float    type; };
template <> struct DataType2Ctype< F64 > { typedef double   type; };
template <> struct DataType2Ctype< B8  > { typedef bool     type; };
template <> struct DataType2Ctype< U8  > { typedef uint8_t  type; };
template <> struct DataType2Ctype< U16 > { typedef uint16_t type; };
template <> struct DataType2Ctype< U32 > { typedef uint32_t type; };
template <> struct DataType2Ctype< U64 > { typedef uint64_t type; };
template <> struct DataType2Ctype< S8  > { typedef int8_t   type; };
template <> struct DataType2Ctype< S16 > { typedef int16_t  type; };
template <> struct DataType2Ctype< S32 > { typedef int32_t  type; };
template <> struct DataType2Ctype< S64 > { typedef int64_t  type; };

/*
 * Ctype
 */
template <DataTypeEnum DT> using Ctype = typename DataType2Ctype< DT >::type;

template <typename ctype> struct Ctype2DataType { static_assert(true,"Invalid Ctype template parameter"); };
template <> struct Ctype2DataType< float    > { static const DataTypeEnum value = F32; };
template <> struct Ctype2DataType< double   > { static const DataTypeEnum value = F64; };
template <> struct Ctype2DataType< bool     > { static const DataTypeEnum value = B8 ; };
template <> struct Ctype2DataType< uint8_t  > { static const DataTypeEnum value = U8 ; };
template <> struct Ctype2DataType< uint16_t > { static const DataTypeEnum value = U16; };
template <> struct Ctype2DataType< uint32_t > { static const DataTypeEnum value = U32; };
template <> struct Ctype2DataType< uint64_t > { static const DataTypeEnum value = U64; };
template <> struct Ctype2DataType< int8_t   > { static const DataTypeEnum value = S8 ; };
template <> struct Ctype2DataType< int16_t  > { static const DataTypeEnum value = S16; };
template <> struct Ctype2DataType< int32_t  > { static const DataTypeEnum value = S32; };
template <> struct Ctype2DataType< int64_t  > { static const DataTypeEnum value = S64; };

/*
 * Dtype
 */
template <typename CT> constexpr DataTypeEnum Dtype() {
	return Ctype2DataType<CT>::value;
}
// How to achieve a shortcut for DTYPE? Maybe using a constexpr function?

template <typename T, typename U>
constexpr bool typeFitsValue(const U value) {
	return ((value>U(0))==(T(value)>T(0))) && U(T(value))==value;
}

} } // namespace map::detail

#endif
