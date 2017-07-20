/* 
 * @file	scalar.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "scalar.hpp"
#include "../runtime/block/Block.hpp"
#include <limits>
#include <cassert>


namespace map { namespace detail {

/***********
   Support
 ***********/

#define SUPPORT template<> const bool IFormat<scalar>::Support

SUPPORT :: StreamDir :: IN  = 1;
SUPPORT :: StreamDir :: OUT = 1;
SUPPORT :: StreamDir :: IO  = 1;

SUPPORT :: DataType :: F32 = 1;
SUPPORT :: DataType :: F64 = 1;
SUPPORT :: DataType :: B8  = 1;
SUPPORT :: DataType :: U8  = 1;
SUPPORT :: DataType :: U16 = 1;
SUPPORT :: DataType :: U32 = 1;
SUPPORT :: DataType :: U64 = 1;
SUPPORT :: DataType :: S8  = 1;
SUPPORT :: DataType :: S16 = 1;
SUPPORT :: DataType :: S32 = 1;
SUPPORT :: DataType :: S64 = 1;

SUPPORT :: NumDim :: TIME = 0;
SUPPORT :: NumDim :: D0 = 1;
SUPPORT :: NumDim :: D1 = 0;
SUPPORT :: NumDim :: D2 = 0;
SUPPORT :: NumDim :: D3 = 0;

SUPPORT :: MemOrder :: BLK = 1;
SUPPORT :: MemOrder :: ROW = 1;
SUPPORT :: MemOrder :: COL = 1;
SUPPORT :: MemOrder :: SFC = 1;

SUPPORT :: Parallel :: PARAREAD = 1;
SUPPORT :: Parallel :: PARAWRITE = 1;

#undef SUPPORT

/***********
   Methods
 ***********/

scalar::scalar(MetaData& meta, DataStats& stats)
	: IFormat<scalar>(meta,stats)
	, type()
	, val()
{ }

scalar::~scalar() { }

Ferr scalar::readBlock(Block *block) const {
	assert(block->holdtype() == HOLD_1);
	block->fixValue(val);
	return 0;
}

Ferr scalar::writeBlock(const Block *block) {
	assert(block->holdtype() == HOLD_1);
	if (type == NONE_REDUCTION) {
		val = block->getValue();
	} else {
		std::lock_guard<std::mutex> lock(mtx); // thread-safe
		val = type.apply(val,block->getValue());
	}
	return 0;
}

Ferr scalar::setReductionType(const ReductionType &type) {
	this->type = type;
	if (type != NONE_REDUCTION) {
		val = type.neutral(meta.getDataType());
	}
	return 0;
}

VariantType scalar::value() const {
	auto ret = val;
	if (type != NONE_REDUCTION) {
		val = type.neutral(meta.getDataType()); // @@ so that multiple evaluations have the value resetted
	}
	return ret;
}

} } // namespace map::detail
