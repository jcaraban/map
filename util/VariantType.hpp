/**
 * @file	VariantType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: study if some constructors could be moved into the Union, w/o drawbacks
 */

#ifndef MAP_UTIL_VARIANT_HPP_
#define MAP_UTIL_VARIANT_HPP_

#include "DataType.hpp"
#include <string>


namespace map { namespace detail {

// Union

union VariantUnion {
	Ctype<F32> f32;
	Ctype<F64> f64;
	Ctype<B8 >  b8;
	Ctype<U8 >  u8;
	Ctype<U16> u16;
	Ctype<U32> u32;
	Ctype<U64> u64;
	Ctype<S8 >  s8;
	Ctype<S16> s16;
	Ctype<S32> s32;
	Ctype<S64> s64;
};

// Class

class VariantType {
	VariantUnion var;
	DataType type;

  public:
  	VariantType();
  	VariantType(VariantUnion var, DataType type);
  	VariantType(Ctype<F32> val);
	VariantType(Ctype<F64> val);
	VariantType(Ctype<B8 > val);
	VariantType(Ctype<U8 > val);
	VariantType(Ctype<U16> val);
	VariantType(Ctype<U32> val);
	VariantType(Ctype<U64> val);
	VariantType(Ctype<S8 > val);
	VariantType(Ctype<S16> val);
	VariantType(Ctype<S32> val);
	VariantType(Ctype<S64> val);
	VariantType(Ctype<F32> val, DataType dt);
	VariantType(Ctype<F64> val, DataType dt);
	VariantType(Ctype<B8 > val, DataType dt);
	VariantType(Ctype<U8 > val, DataType dt);
	VariantType(Ctype<U16> val, DataType dt);
	VariantType(Ctype<U32> val, DataType dt);
	VariantType(Ctype<U64> val, DataType dt);
	VariantType(Ctype<S8 > val, DataType dt);
	VariantType(Ctype<S16> val, DataType dt);
	VariantType(Ctype<S32> val, DataType dt);
	VariantType(Ctype<S64> val, DataType dt);

  	DataType datatype() const;
  	bool isEqual(VariantType other) const;
  	bool operator==(VariantType other) const;
  	bool operator!=(VariantType other) const;
  	size_t hash() const;
  	std::string toString() const;

  	VariantUnion& ref();
  	VariantUnion get() const;
	void set(VariantUnion var, DataType dt);
	template <DataTypeEnum T> Ctype<T>& ref();
	template <DataTypeEnum T> Ctype<T> get() const;
  	template <typename T> void set(T val, DataType dt);
  	
  	VariantType convert(DataType dt);

  	bool isNone() const;
  	bool isZero() const;
  	bool isOne() const;

  	friend std::ostream& operator<<(std::ostream &strm, const VariantType &var);
};

static_assert( std::is_standard_layout< VariantType >::value , "VariantType must be C compatible");

/*
 *
 */
struct ValFix {
	VariantType value;
	bool fixed;
};

} } // namespace map::detail

#endif

#include "VariantType.tpl"
