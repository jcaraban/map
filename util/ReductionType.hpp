/**
 * @file	ReductionType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: find better names for rAND / rOR? also called logical CONjunction / logical DISjunction
 * TODO: add MEAN enum, which would behave like an online variance calculation
 * TODO: finish the implementation of 'atomic' or remove it all together
 */

#ifndef MAP_UTIL_REDUCTIONTYPE_HPP_
#define MAP_UTIL_REDUCTIONTYPE_HPP_

#include "VariantType.hpp"
#include <string>
#include <tuple>


namespace map { namespace detail {

/*
 *Enum
 */
enum ReductionEnum : int {
	NONE_REDUCTION, SUM, PROD, rAND, rOR, MARK_REDUCTION, MAX, MIN, N_REDUCTION
};

ReductionEnum operator ++ (ReductionEnum& rte);


/*
 * Class
 */
class ReductionType {
	ReductionEnum type;

  public:
  	ReductionType();
  	ReductionType(ReductionEnum type);
  	ReductionEnum get() const;

  	bool operator==(ReductionType type) const;
  	bool operator!=(ReductionType type) const;

  	bool isOperator() const;
  	bool isFunction() const;
	std::string toString() const;
	std::string code() const;
	std::string neutralString(DataType dt) const;
	VariantType neutral(DataType dt) const;
	VariantType apply(VariantType lhs, VariantType rhs) const;
	void atomic(VariantType &lhs, VariantType rhs) const;

  private:
	template <ReductionEnum R> VariantType neutral1(DataType dt) const ;
	template <ReductionEnum R, DataTypeEnum T> VariantType neutral2() const ;
	
	template <ReductionEnum R> VariantType apply1(VariantType lhs, VariantType rhs) const ;
	template <ReductionEnum R, DataTypeEnum T> VariantType apply2(VariantType lhs, VariantType rhs) const ;

	template <ReductionEnum R> void atomic1(VariantType &lhs, VariantType rhs) const ;
	template <ReductionEnum R, DataTypeEnum T> void atomic2(VariantType &lhs, VariantType rhs) const ;
};

static_assert( std::is_standard_layout< ReductionType >::value , "ReductionType must be C compatible");

} } // namespace map::detail

#endif

#include "ReductionType.tpl"
