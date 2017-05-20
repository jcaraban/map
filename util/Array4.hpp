/**
 * @file	Array4.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Static version of Array, with up to 4 elements
 *
 * TODO: making the class 'trivially copiable' might help vectorization
 * TODO: could #define PROMOTE be changed with a constexpr function?
 */

#ifndef MAP_UTIL_ARRAY4_HPP_
#define MAP_UTIL_ARRAY4_HPP_

#include "promote.hpp"
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <initializer_list>
#include <type_traits>
 
//#define NDEBUG
#include <cassert>


namespace map { namespace detail {

namespace { // anonymous namespace
	template <typename T> struct defval { static const T value = -1;};
	template <> struct defval<bool> { static const bool value = false; };
}

/*
 * @class Meta-class Array4<>
 *
 */
template <typename T>
class Array4
{
	static_assert(std::is_arithmetic<T>::value,"Templated type must be of arithmetic type");

  protected:
	static const int N = 2; // @
	static const T def = defval<T>::value;
	std::array<T,N> arr;
	int n;

  public:
	Array4();
	//Array4(T x);
	//Array4(T x, T y);
	//Array4(T x, T y, T z);
	//Array4(T x, T y, T z, T t);

	/*
	 *
	 */
	explicit Array4(int n);

	/*
	 *
	 */
	Array4(int n, const T &val);

	/*
	 *
	 */
	Array4(std::initializer_list<T> ini_list);

	/*
	 *
	 */
	//Array4& operator=(const T& val);

	/*
	 *
	 */
	template <typename OT>
	Array4& operator=(const Array4<OT>& other);

	/*
	 *
	 */
	template <typename OT>
	operator Array4<OT>() const;

	/*
	 *
	 */
	template <typename OT>
	Array4<OT> promote() const;

	/*
	 *
	 */
	T& operator[](int i);
	const T& operator[](int i) const;

	/*
	 *
	 */
	int size() const;

	/*
	 *
	 */
	bool isNone() const;
};

static_assert( std::is_standard_layout< Array4<int> >::value , "Array4 must be C compatible");

/*
 * This types are public because they can be utilized by the user
 */
using DataSize = Array4<int>; // @ change to 64b?
using BlockSize = Array4<int>; // is 32b ok?
using NumBlock = Array4<int>; // is 32b ok?
using GroupSize = Array4<int>; // is 16b ok?
using NumGroup = Array4<int>; // is 16b ok?
using Coord = Array4<int>; // data_coord -> 64, block_coord -> 32, group_coord -> 16 ?

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_UTIL_ARRAY4_TPL_
#define MAP_UTIL_ARRAY4_TPL_

namespace map { namespace detail {

template <typename T>
Array4<T>::Array4() :
	arr{def,def},//,def,def}
	n(def)
{ }
/*
template <typename T>
Array4<T>::Array4(T x) :
	arr{x,def},//,def,def}
	n(1)
{ }

template <typename T>
Array4<T>::Array4(T x, T y) :
	arr{x,y},//,def,def}
	n(2)
{ }

template <typename T>
Array4<T>::Array4(T x, T y, T z) :
	arr{x,y,z,def},
	n(3)
{ }

template <typename T>
Array4<T>::Array4(T x, T y, T z, T t) :
	arr{x,y,z,t}
	n(4)
{ }
*/
template <typename T>
Array4<T>::Array4(int n) :
	arr{def,def},
	n(n)
{
	assert(n <= N);
}

template <typename T>
Array4<T>::Array4(int n, const T &val) :
	arr{},
	n(n)
{
	assert(n <= N);
	for (int i=0; i<size(); i++)
		arr[i] = val;
}

template <typename T>
Array4<T>::Array4(std::initializer_list<T> ini_list) :
	arr{},
	n(ini_list.size())
{
	assert(n <= N);
	auto it = ini_list.begin();
	for (int i=0; i<size(); i++, it++)
		arr[i] = *it;
}
/*
template <typename T>
Array4<T>& Array4<T>::operator=(const T& val) {
	for (int i=0; i<size(); i++)
		arr[i] = val;
	return *this;
}
*/
// Other type cast

template <typename T>
template <typename OT>
Array4<T>& Array4<T>::operator=(const Array4<OT>& other) {
	n = other.size();
	for (int i=0; i<size(); i++)
		arr[i] = static_cast<T>(other[i]);
	return *this;
}

template <typename T>
template <typename OT>
Array4<T>::operator Array4<OT>() const {
	Array4<OT> new_array4(size());
	for (int i=0; i<size(); i++)
		new_array4[i] = static_cast<OT>(arr[i]);
	return new_array4;
}

template <typename T>
template <typename OT>
Array4<OT> Array4<T>::promote() const {
	Array4<OT> new_array4(size());
	for (int i=0; i<size(); i++)
		new_array4[i] = static_cast<OT>(arr[i]);
	return new_array4;
}

// Accessors

template <typename T>
T& Array4<T>::operator[](int i) {
	return arr[i];
}

template <typename T>
const T& Array4<T>::operator[](int i) const {
	return arr[i];
}

template <typename T>
int Array4<T>::size() const {
	return n;
}

template <typename T>
bool Array4<T>::isNone() const {
	return size() < 0;
}

/*************
   Operators
 *************/

//#define ARITHTYPE(T) typename std::enable_if<std::is_arithmetic<T>::value,T>::type

#define DEFINE_ARRAY4_COMPOUND_ASSIGNMENT_OPERATOR(op) \
	template <typename T, typename OT> \
	Array4<T>& operator op (Array4<T>& target, const Array4<OT>& other) { \
		assert(target.size()==other.size()); \
		for (int i=0; i<target.size(); i++) \
			target[i] op other[i]; \
		return target; \
	} \
	template <typename T, typename OT> \
	Array4<T>& operator op (Array4<T>& target, const OT& val) { \
		for (int i=0; i<target.size(); i++) \
			target[i] op val; \
		return target; \
	}

	DEFINE_ARRAY4_COMPOUND_ASSIGNMENT_OPERATOR(+=)
	DEFINE_ARRAY4_COMPOUND_ASSIGNMENT_OPERATOR(-=)
	DEFINE_ARRAY4_COMPOUND_ASSIGNMENT_OPERATOR(*=)
	DEFINE_ARRAY4_COMPOUND_ASSIGNMENT_OPERATOR(/=)
	DEFINE_ARRAY4_COMPOUND_ASSIGNMENT_OPERATOR(%=)
#undef DEFINE_ARRAY4_COMPOUND_ASSIGNMENT_OPERATOR

#define PROMOTE Ctype< Promote< Ctype2DataType<T>::value , Ctype2DataType<OT>::value >::value >

#define DEFINE_ARRAY4_BINARY_ARITHMETIC_OPERATOR(op,base_op) \
	template <typename T, typename OT> \
	Array4<PROMOTE> operator op (const Array4<T>& lhs, const Array4<OT>& rhs) { \
		if (lhs.size()==0) \
			return rhs.operator Array4<PROMOTE>(); \
		if (rhs.size()==0) \
			return lhs.operator Array4<PROMOTE>(); \
		assert(lhs.size()==rhs.size()); \
		const int size = lhs.size(); \
		auto left_array4 = lhs.operator Array4<PROMOTE>(); \
		auto right_array4 = rhs.operator Array4<PROMOTE>(); \
		left_array4 base_op right_array4; \
		return left_array4; \
	} \
	template <typename T, typename OT> \
	Array4<PROMOTE> operator op (const Array4<T>& lhs, const OT& rhs) { \
		const int size = lhs.size(); \
		auto left_array4 = lhs.operator Array4<PROMOTE>(); \
		auto right_array4 = Array4<PROMOTE>(lhs.size(),rhs); \
		left_array4 base_op right_array4; \
		return left_array4; \
	} \
	template <typename T, typename OT> \
	Array4<PROMOTE> operator op (const OT& lhs, const Array4<T>& rhs) { \
		const int size = rhs.size(); \
		auto left_array4 = Array4<PROMOTE>(rhs.size(),lhs); \
		auto right_array4 = rhs.operator Array4<PROMOTE>(); \
		left_array4 base_op right_array4; \
		return left_array4; \
	}

	DEFINE_ARRAY4_BINARY_ARITHMETIC_OPERATOR(+,+=)
	DEFINE_ARRAY4_BINARY_ARITHMETIC_OPERATOR(-,-=)
	DEFINE_ARRAY4_BINARY_ARITHMETIC_OPERATOR(*,*=)
	DEFINE_ARRAY4_BINARY_ARITHMETIC_OPERATOR(/,/=)
	DEFINE_ARRAY4_BINARY_ARITHMETIC_OPERATOR(%,%=)
#undef DEFINE_ARRAY4_BINARY_ARITHMETIC_OPERATOR

#define DEFINE_ARRAY4_RELATIONAL_OPERATOR(op) \
	template <typename T, typename OT> \
	Array4<bool> operator op (const Array4<T> &lhs, const Array4<OT> &rhs) { \
		assert(lhs.size()==rhs.size()); \
		const int size = lhs.size(); \
		Array4<PROMOTE> left_array4(size), right_array4(size); \
		Array4<bool> bool_array4(size); \
		for (int i=0; i<size; i++) \
			left_array4[i] = lhs[i]; \
		for (int i=0; i<size; i++) \
			right_array4[i] = rhs[i]; \
		for (int i=0; i<size; i++) \
			bool_array4[i] = left_array4[i] op right_array4[i]; \
		return bool_array4; \
	} \
	template <typename T, typename OT> \
	Array4<bool> operator op (const Array4<T> &lhs, const OT &rhs) { \
		const int size = lhs.size(); \
		Array4<PROMOTE> left_array4(size), right_array4(size); \
		Array4<bool> bool_array4(size); \
		for (int i=0; i<lhs.size(); i++) \
			left_array4[i] = lhs[i]; \
		for (int i=0; i<lhs.size(); i++) \
			right_array4[i] = rhs; \
		for (int i=0; i<lhs.size(); i++) \
			bool_array4[i] = left_array4[i] op right_array4[i]; \
		return bool_array4; \
	} \
	template <typename T, typename OT> \
	Array4<bool> operator op (const OT &lhs, const Array4<T> &rhs) { \
		const int size = rhs.size(); \
		Array4<PROMOTE> left_array4(size), right_array4(size); \
		Array4<bool> bool_array4(size); \
		for (int i=0; i<rhs.size(); i++) \
			left_array4[i] = lhs; \
		for (int i=0; i<rhs.size(); i++) \
			right_array4[i] = rhs[i]; \
		for (int i=0; i<lhs.size(); i++) \
			bool_array4[i] = left_array4[i] op right_array4[i]; \
		return bool_array4; \
	}

	DEFINE_ARRAY4_RELATIONAL_OPERATOR(==)
	DEFINE_ARRAY4_RELATIONAL_OPERATOR(!=)
	DEFINE_ARRAY4_RELATIONAL_OPERATOR(< )
	DEFINE_ARRAY4_RELATIONAL_OPERATOR(<=)
	DEFINE_ARRAY4_RELATIONAL_OPERATOR(> )
	DEFINE_ARRAY4_RELATIONAL_OPERATOR(>=)
	DEFINE_ARRAY4_RELATIONAL_OPERATOR(&&)
	DEFINE_ARRAY4_RELATIONAL_OPERATOR(||)
#undef DEFINE_ARRAY4_RELATIONAL_OPERATOR

//#undef ARITHTYPE
#undef PROMOTE

template <typename T>
Array4<bool> operator!(const Array4<T>& target) {
	Array4<bool> new_array4(target.size());
	for (int i=0; i<target.size(); i++)
		new_array4[i] = ! static_cast<bool>(target[i]);
	return new_array4;
}

/***********
   Methods
 ***********/

#define DEFINE_ARRAY4_ELEMENT_WISE_FUNCTION(name,func) \
	template <typename T> \
	Array4<T> name(const Array4<T> &array4) { \
		Array4<T> new_array4(array4.size()); \
		for (int i=0; i<array4.size(); i++) \
			new_array4[i] = func(array4[i]); \
		return new_array4; \
	}

	DEFINE_ARRAY4_ELEMENT_WISE_FUNCTION(abs,std::abs)
	DEFINE_ARRAY4_ELEMENT_WISE_FUNCTION(floor,std::floor)
	DEFINE_ARRAY4_ELEMENT_WISE_FUNCTION(ceil,std::ceil)
#undef DEFINE_ARRAY4_ELEMENT_WISE_FUNCTION

#define DEFINE_ARRAY4_REDUCTION_OPERATOR(name,op,init_val) \
	template <typename T> \
	T name(const Array4<T> &array4) { \
		T acu = init_val; \
		for (int i=0; i<array4.size(); i++) \
			acu = acu op array4[i]; \
		return acu; \
	}

	DEFINE_ARRAY4_REDUCTION_OPERATOR(all,&&,true)
	DEFINE_ARRAY4_REDUCTION_OPERATOR(any,||,false)
	DEFINE_ARRAY4_REDUCTION_OPERATOR(sum,+,0)
	DEFINE_ARRAY4_REDUCTION_OPERATOR(prod,*,1)
#undef DEFINE_ARRAY4_REDUCTION_OPERATOR

#define DEFINE_ARRAY4_REDUCTION_FUNCTION(name,func,init_val) \
	template <typename T> \
	T name(const Array4<T> &array4) { \
		T acu = init_val; \
		for (int i=0; i<array4.size(); i++) \
			acu = func( acu , array4[i] ); \
		return acu; \
	}

	DEFINE_ARRAY4_REDUCTION_FUNCTION(max,std::max,array4[0])
	DEFINE_ARRAY4_REDUCTION_FUNCTION(min,std::min,array4[0])
#undef DEFINE_ARRAY4_REDUCTION_FUNCTION

template <typename T>
Array4<T> max(const Array4<T> &left_array4, const Array4<T> &right_array4) {
	Array4<T> new_array4(left_array4.size());
	for (int i=0; i<new_array4.size(); i++)
		new_array4[i] = (left_array4[i] > right_array4[i]) ? left_array4[i] : right_array4[i];
	return new_array4;
}

template <typename T>
Array4<T> min(const Array4<T> &left_array4, const Array4<T> &right_array4) {
	Array4<T> new_array4(left_array4.size());
	for (int i=0; i<new_array4.size(); i++)
		new_array4[i] = (left_array4[i] < right_array4[i]) ? left_array4[i] : right_array4[i];
	return new_array4;
}

template <typename T>
T norm(const Array4<T> &array4) {
	T acu = 0;
	for (int i=0; i<array4.size(); i++)
		acu = acu + array4[i]*array4[i];
	return std::sqrt(acu);
}

template <typename T>
Array4<T> sgn(const Array4<T> &array) {
	Array4<T> new_array(array.size());
	for (int i=0; i<array.size(); i++)
		new_array[i] = (T(0) < array[i]) - (array[i] < T(0));
	return new_array;
}

template <typename T>
Array4<T> cond(const Array4<bool> &cond, const Array4<T> &lhs, const Array4<T> &rhs) {
	assert(cond.size() == lhs.size() && lhs.size() == rhs.size());
	Array4<T> new_array(cond.size());
	for (int i=0; i<cond.size(); i++)
		new_array[i] = cond[i] ? lhs[i] : rhs[i];
	return new_array;
}

template <typename T>
std::string to_string(const Array4<T> &array) {
	assert(not array.isNone());
	std::string str = "{";
	for (int i=0; i<array.size(); i++) {
		std::string comma = (i < array.size()-1) ? "," : "";
		str += std::to_string(array[i]) + comma;
	}
	str += "}";
	return str;
}

template <typename T>
std::ostream& operator<<(std::ostream &strm, const Array4<T> &array4) {
	return strm << to_string(array4);
}


/**********
   Coord
 **********/

inline int proj(const Coord &coord, const DataSize &dim_size) {
	if (coord.size() == 0)
		return 0; // @
	// Recursively ((z_coord)*y_size + y_coord)*x_size + x_coord
	int acu = coord[ coord.size()-1 ];
	for (int i=coord.size()-2; i>=0; i--) {
		acu = acu * dim_size[i];
		acu += coord[i];
	}
	return acu;
}

inline int proj(const Coord &coord, const DataSize &dim_size, const Coord &offset) {
	if (coord.size() == 0)
		return 0;
	// Recursively ((z_coord + z_off)*y_size + y_coord + y_off)*x_size + x_coord + x_off
	int acu = coord[ coord.size()-1 ] + offset[ coord.size()-1 ];
	for (int i=coord.size()-2; i>=0; i--) {
		acu = acu * dim_size[i];
		acu += coord[i] + offset[i];
	}
	return acu;
}

inline Array4<bool> in_range(const Coord &coord, const DataSize &dim_size) {
	if (coord.size() == 0)
		return Array4<bool>(coord.size(),true); // @
	//
	Array4<bool> array4(coord.size());
	for (int i=0; i<coord.size(); i++)
		array4[i] = (coord[i] >= 0 && coord[i] < dim_size[i]);
	return array4;
}

inline Coord next(Coord coord, Coord begin, Coord end, int step=1) {
	assert(coord.size() == begin.size());
	assert(begin.size() == end.size());
	assert(not coord.isNone());
	assert(all(begin <= end));
	assert(any(coord < end));
	int carry = 0;

	for (int i=0; i<coord.size(); i++) {
		coord[i] += (i == 0) ? step : carry;
		if (coord[i] < end[i])
			return coord;
		carry = step;
		coord[i] = begin[i];
	}
	return end;
}

inline std::vector<Coord> iterSpace(Coord begin, Coord end) {
	assert(begin.size() == end.size());
	if (begin.size() == 0)
		return std::vector<Coord>(1,Coord(0));

	std::vector<Coord> vec;
	Coord coord = begin;

	while (all(coord < end)) {
		vec.push_back(coord);
		coord = next(coord,begin,end);
	}
	return vec;
}

inline bool neighbors(Coord lhs, Coord rhs, int step=1) {
	assert(lhs.size() == rhs.size());
	assert(!lhs.isNone());

	for (int i=0; i<lhs.size(); i++)
		if (std::abs(lhs[i]-rhs[i]) > step)
			return false;
	return true;
}

struct coord_hash {
	std::size_t operator()(const Coord &c) const {
		std::size_t h = 0;
		for (int i=0; i<c.size(); i++)
			h ^= std::hash<int>()(c[i]);
		return h;
	}
};

struct coord_equal {
	bool operator()(const Coord &lhs, const Coord &rhs) const {
		return lhs.size() == rhs.size() && all(lhs == rhs);
	}
};

} } // namespace map::detail

#undef NDEBUG

#endif
