/**
 * @file	Array.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: Array is visible to the user (map::Array)
 *
 * TODO: no need for mutability, since the interface is now Python
 */

#ifndef MAP_UTIL_ARRAY_HPP_
#define MAP_UTIL_ARRAY_HPP_

#include "DataType.hpp"
#include "NumDim.hpp"
#include "promote.hpp"
#include "Array4.hpp"
#include <iostream>
#include <vector>
#include <initializer_list>
#include <type_traits>
#include <cassert>


namespace map { namespace detail {

namespace { // anonymous namespace
	// Template filter to avoid the specialized version of std::vector<bool>
	template <typename T> struct no_bool { typedef T type; };
	template <> struct no_bool<bool> { typedef char type; };
	template <typename T> using NoBool = typename no_bool<T>::type;
}

/*
 * @class Array
 * Wrapper of an stl::vector with extra functionalities
 *
 * This class is necessary to ensure consistency with Array4<>, which is necessary for performance purposes
 */
template <typename T>
class Array
{
  protected:
	mutable std::vector< NoBool<T> > vec;

  public:

	/*
	 *
	 */
	Array();

	/*
	 *
	 */
	explicit Array(int n);

	/*
	 *
	 */
	Array(int n, const T &val);

	/*
	 *
	 */
	Array(std::initializer_list<T> ini_list);

	/*
	 *
	 */
	~Array();

	/*
	 *
	 */
	Array(const Array &other);

	/*
	 *
	 */
	Array& operator=(const Array &other);

	/*
	 *
	 */
	Array(const Array &&other);

	/*
	 *
	 */
	Array& operator=(const Array &&other);

	/*
	 *
	 */
	Array& operator=(std::initializer_list<T>);

	/*
	 * Type cast
	 */
	template <typename OT>
	operator Array<OT>() const;

	/*
	 * Low-High cast
	 */
	//Array(const Array4<T> &array);

	/*
	 * Low-High cast
	 */
	//Array& operator=(const Array4<T> &array);

	/*
	 * High-Low cast
	 */
	//operator Array4<T>() const;

	/*
	 * Accessors
	 */
	NoBool<T>& operator[](int i);
	
	/*
	 * Accessors
	 */
	const NoBool<T>& operator[](int i) const;
	
	/*
	 * Size
	 */
	int size() const;

	/*
	 * Returns 'true' if Array is empty
	 */
	bool isNone() const;

	Array& operator+=(const Array &other);
	Array& operator-=(const Array &other);
	Array& operator*=(const Array &other);
	Array& operator/=(const Array &other);
	Array& operator%=(const Array &other);
	Array& operator+=(const T &val);
	Array& operator-=(const T &val);
	Array& operator*=(const T &val);
	Array& operator/=(const T &val);
	Array& operator%=(const T &val);
	Array<NoBool<bool>> operator!();
};

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_UTIL_ARRAY_TPL_
#define MAP_UTIL_ARRAY_TPL_

namespace map { namespace detail {

template <typename T>
Array<T>::Array()
	: vec( )
{ }

template <typename T>
Array<T>::Array(int n)
	: vec( n )
{ }

template <typename T>
Array<T>::Array(int n, const T &val)
	: vec(n,val)
{ }

template <typename T>
Array<T>::Array(std::initializer_list<T> ini_list)
	//: vec( ini_list ) otherwise NoBools breaks
{
	for (auto elem : ini_list)
		vec.push_back(elem);
}

template <typename T>
Array<T>::~Array() { }

template <typename T>
Array<T>::Array(const Array<T> &other)
	: vec( other.vec )
{ }

template <typename T>
Array<T>& Array<T>::operator=(const Array<T> &other) {
	vec = other.vec;
	return *this;
}

template <typename T>
Array<T>::Array(const Array<T> &&other)
	: vec( other.vec )
{ }

template <typename T>
Array<T>& Array<T>::operator=(const Array<T> &&other) {
	vec = other.vec;
	return *this;
}

template <typename T>
Array<T>& Array<T>::operator=(std::initializer_list<T> ini_list) {
	vec = ini_list;
	return *this;
}

// Type cast

template <typename T>
template <typename OT>
Array<T>::operator Array<OT>() const {
	Array<OT> new_array;
	for (int i=0; i<size(); i++)
		new_array[i] = static_cast<OT>(vec[i]);
	return new_array;
}

// High-Low cast
/*
template <typename T>
Array<T>::Array(const Array4<T> &array4) {
	vec.resize(array4.size());
	for (int i=0; i<size(); i++)
		vec[i] = array4[i];
}

template <typename T>
Array<T>& Array<T>::operator=(const Array4<T> &array4) {
	vec.resize(array4.size());
	for (int i=0; i<size(); i++)
		vec[i] = array4[i];
	return *this;
}

template <typename T>
Array<T>::operator Array4<T>() const {
	Array4<T> array4;
	for (int i=0; i<size(); i++)
		array4[i] = vec[i];
	return array4;
}
*/
// Accessors

template <typename T>
NoBool<T>& Array<T>::operator[](int i) {
	if (i >= (int)vec.size())
		vec.resize(i+1);
	return vec[i];
}

template <typename T>
const NoBool<T>& Array<T>::operator[](int i) const {
	if (i >= (int)vec.size())
		vec.resize(i+1);
	// vec is mutable because it is resized when new positions are accessed
	return vec[i];
}

template <typename T>
int Array<T>::size() const {
	return vec.size();
}

template <typename T>
bool Array<T>::isNone() const {
	return vec.empty();
}

/*************
   Operators
 *************/

#define DEFINE_ARRAY_COMPOUND_ASSIGNMENT_OPERATOR(op) \
	template <typename T> \
	Array<T>& Array<T>::operator op (const Array<T> &other) { \
		assert(size() == other.size()); \
		for (int i=0; i<size(); i++) \
			vec[i] op other.vec[i]; \
		return *this; \
	} \
	template <typename T> \
	Array<T>& Array<T>::operator op (const T &val) { \
		for (int i=0; i<size(); i++) \
			vec[i] op val; \
		return *this; \
	}

	DEFINE_ARRAY_COMPOUND_ASSIGNMENT_OPERATOR(+=)
	DEFINE_ARRAY_COMPOUND_ASSIGNMENT_OPERATOR(-=)
	DEFINE_ARRAY_COMPOUND_ASSIGNMENT_OPERATOR(*=)
	DEFINE_ARRAY_COMPOUND_ASSIGNMENT_OPERATOR(/=)
	DEFINE_ARRAY_COMPOUND_ASSIGNMENT_OPERATOR(%=)
#undef DEFINE_ARRAY_COMPOUND_ASSIGNMENT_OPERATOR

#define PROMOTE Ctype< Promote< Ctype2DataType<T>::value , Ctype2DataType<OT>::value >::value >

#define DEFINE_ARRAY_BINARY_ARITHMETIC_OPERATOR(op,base_op) \
	template <typename T, typename OT> \
	Array<PROMOTE> operator op (const Array<T> &lhs, const Array<OT> &rhs) { \
		assert(lhs.size() == rhs.size()); \
		Array<PROMOTE> left_array(lhs), right_array(rhs); \
		left_array base_op right_array; \
		return left_array; \
	} \
	template <typename T, typename OT> \
	Array<PROMOTE> operator op (const Array<T> &lhs, const OT &rhs) { \
		Array<PROMOTE> left_array(lhs), right_array(lhs.size(),rhs); \
		left_array base_op right_array; \
		return left_array; \
	} \
	template <typename T, typename OT> \
	Array<PROMOTE> operator op (const OT &lhs, const Array<T> &rhs) { \
		Array<PROMOTE> left_array(rhs.size(),lhs), right_array(rhs); \
		left_array base_op right_array; \
		return left_array; \
	}

	DEFINE_ARRAY_BINARY_ARITHMETIC_OPERATOR(+,+=)
	DEFINE_ARRAY_BINARY_ARITHMETIC_OPERATOR(-,-=)
	DEFINE_ARRAY_BINARY_ARITHMETIC_OPERATOR(*,*=)
	DEFINE_ARRAY_BINARY_ARITHMETIC_OPERATOR(/,/=)
	DEFINE_ARRAY_BINARY_ARITHMETIC_OPERATOR(%,%=)
#undef DEFINE_ARRAY_BINARY_ARITHMETIC_OPERATOR

#define DEFINE_ARRAY_RELATIONAL_OPERATOR(op) \
	template <typename T, typename OT> \
	Array<NoBool<bool>> operator op (const Array<T> &lhs, const Array<OT> &rhs) { \
		assert(lhs.size() == rhs.size()); \
		Array<PROMOTE> left_array(lhs), right_array(rhs); \
		Array<NoBool<bool>> new_array; \
		for (int i=0; i<lhs.size(); i++) \
			new_array[i] = left_array[i] op right_array[i]; \
		return new_array; \
	} \
	template <typename T, typename OT> \
	Array<NoBool<bool>> operator op (const Array<T> &lhs, const OT &rhs) { \
		Array<PROMOTE> left_array(lhs), right_array(lhs.size(),rhs); \
		Array<NoBool<bool>> new_array; \
		for (int i=0; i<lhs.size(); i++) \
			new_array[i] = left_array[i] op right_array[i]; \
		return new_array; \
	} \
	template <typename T, typename OT> \
	Array<NoBool<bool>> operator op (const OT &lhs, const Array<T> &rhs) { \
		Array<PROMOTE> left_array(rhs.size(),lhs), right_array(rhs); \
		Array<NoBool<bool>> new_array; \
		for (int i=0; i<rhs.size(); i++) \
			new_array[i] = left_array[i] op right_array[i]; \
		return new_array; \
	}

	DEFINE_ARRAY_RELATIONAL_OPERATOR(==)
	DEFINE_ARRAY_RELATIONAL_OPERATOR(!=)
	DEFINE_ARRAY_RELATIONAL_OPERATOR(< )
	DEFINE_ARRAY_RELATIONAL_OPERATOR(<=)
	DEFINE_ARRAY_RELATIONAL_OPERATOR(> )
	DEFINE_ARRAY_RELATIONAL_OPERATOR(>=)
	DEFINE_ARRAY_RELATIONAL_OPERATOR(&&)
	DEFINE_ARRAY_RELATIONAL_OPERATOR(||)
#undef DEFINE_ARRAY_RELATIONAL_OPERATOR

#undef PROMOTE
#undef PROMOTE

template <typename T>
Array<NoBool<bool>> Array<T>::operator!() {
	Array<NoBool<bool>> new_array;
	for (int i=0; i<size(); i++)
		new_array[i] = ! static_cast<bool>(vec[i]);
	return new_array;
}

/**********
   Others
 **********/

#define DEFINE_ARRAY_ELEMENT_WISE_FUNCTION(name,func) \
	template <typename T> \
	Array<T> name(const Array<T> &array) { \
		Array<T> new_array; \
		for (int i=0; i<array.size(); i++) \
			new_array[i] = func(array[i]); \
		return new_array; \
	}

	DEFINE_ARRAY_ELEMENT_WISE_FUNCTION(abs,std::abs)
	DEFINE_ARRAY_ELEMENT_WISE_FUNCTION(floor,std::floor)
	DEFINE_ARRAY_ELEMENT_WISE_FUNCTION(ceil,std::ceil)
	//DEFINE_ARRAY_ELEMENT_WISE_FUNCTION(sgn, [](T t){ return (T(0) < t) - (t < T(0)); })
#undef DEFINE_ARRAY_ELEMENT_WISE_FUNCTION

#define DEFINE_ARRAY_REDUCTION_FUNCTION(name,op,neutral) \
	template <typename T> \
	T name(const Array<T> &array) { \
		T acu = neutral; \
		for (int i=0; i<array.size(); i++) \
			acu = acu op array[i]; \
		return acu; \
	}

	DEFINE_ARRAY_REDUCTION_FUNCTION(all,&&,true) // 'and' is a reserved word, using 'all'
	DEFINE_ARRAY_REDUCTION_FUNCTION(any,||,false) // 'or' is a reserved word, using 'any'
	DEFINE_ARRAY_REDUCTION_FUNCTION(sum,+,0)
	DEFINE_ARRAY_REDUCTION_FUNCTION(prod,*,1)
#undef DEFINE_ARRAY_REDUCTION_FUNCTION

template <typename T>
Array<T> sgn(const Array<T> &array) {
	Array<T> new_array;
	for (int i=0; i<array.size(); i++)
		new_array[i] = (T(0) < array[i]) - (array[i] < T(0));
	return new_array;
}

template <typename T>
std::string to_string(const Array<T> &array) {
	std::string str = "{";
	for (int i=0; i<array.size(); i++) {
		std::string comma = (i < array.size()-1) ? "," : "";
		str += std::to_string(array[i]) + comma;
	}
	str += "}";
	return str;
}

template <typename T>
std::ostream& operator<<(std::ostream &strm, const Array<T> &array) {
	return strm << to_string(array);
}

template <typename T>
Array<T> cond(const Array<NoBool<bool>> &cond, const Array<T> &lhs, const Array<T> &rhs) {
	assert(cond.size() == lhs.size() && lhs.size() == rhs.size());
	Array<T> new_array;
	for (int i=0; i<cond.size(); i++)
		new_array[i] = cond[i] ? lhs[i] : rhs[i];
	return new_array;
}

} } // namespace map::detail

#endif
