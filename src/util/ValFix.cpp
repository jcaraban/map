/**
 * @file	ValFix.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "ValFix.hpp"


namespace map { namespace detail {

ValFix::ValFix()
	: value()
	, min()
	, max()
	, mean()
	, std()
	, fixed(false)
	, active(false)
{ }

ValFix::ValFix(VariantType val)
	: value(val)
	, min(val)
	, max(val)
	, mean(val)
	, std(val)
	, fixed(true)
	, active(true)
{ }
/*
ValFix::ValFix(CellStats sta)
	: value()
	, min(VariantType(sta.min, data_type ), max(sta.max), mean(sta.mean), std(sta.std), fixed(false), active(true)
{ }
*/
ValFix::ValFix(VariantType val, bool fix, CellStats sta) {
	if (fix) {
		value = val;
		min = val;
		max = val;
		mean = val;
		std = val;
		fixed = true;
		active = true;
	} else if (sta.active) {
		value = VariantType();
		min = sta.min;
		max = sta.max;
		mean = sta.mean;
		std = sta.std;
		fixed = false;
		active = true;
	} else {
		value = VariantType();
		min = VariantType();
		max = VariantType();
		mean = VariantType();
		std = VariantType();
		fixed = false;
		active = false;
	}
	assert(fixed || val.isNone());
}

CellStats ValFix::stats() const {
	CellStats sta;
	sta.active = active;
	sta.data_type = min.datatype();
	sta.min = min;
	sta.max = max;
	sta.mean = mean;
	sta.std = std;
	return sta;
}

} } // namespace map::detail
