/**
 * @file	DataStats.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "DataStats.hpp"


namespace map { namespace detail {

// CellStats

CellStats::CellStats()
	: active(false)
	, data_type()
	, min()
	, max()
	, mean()
	, std()
{ }

// DataStats

DataStats::DataStats()
	: active(false)
	, data_type()
	, num_block()
	, min()
	, max()
	, mean()
	, std()
	, minb()
	, maxb()
	, meanb()
	, stdb()
{ }

CellStats DataStats::get(Coord coord) const {
	CellStats stats;
	int idx = proj(coord,num_block);

	stats.active = true;
	stats.data_type = data_type;
	stats.min = VariantType(minb[idx],data_type);
	stats.max = VariantType(maxb[idx],data_type);
	stats.mean = VariantType(meanb[idx],data_type);
	stats.std = VariantType(stdb[idx],data_type);

	return stats;
}

void DataStats::set(Coord coord, CellStats stats) {
	assert(stats.active);
	assert(data_type == stats.data_type);
	int idx = proj(coord,num_block);

	minb[idx] = stats.min.get();
	maxb[idx] = stats.max.get();
	meanb[idx] = stats.mean.get();
	stdb[idx] = stats.std.get();
}

// BlockStats

BlockStats::BlockStats()
	: active(false)
	, data_type()
	, num_group()
	, min()
	, max()
	, mean()
	, std()
	, ming()
	, maxg()
	, meang()
	, stdg()
{ }

bool BlockStats::operator==(const BlockStats &other) {
	bool b = true;
	b = b && data_type == other.data_type;
	b = b && all(num_group == other.num_group);
	if (data_type != NONE_DATATYPE) {
		b = b && min == other.min;
		b = b && max == other.max;
		b = b && mean == other.mean;
		b = b && std == other.std;
	}
	return b;
}

BlockStats::operator CellStats() const {
	CellStats cell;
	cell.active = active;
	cell.data_type = data_type;
	cell.min = min;
	cell.max = max;
	cell.mean = mean;
	cell.std = std;
	return cell;
}

//

CellStats defaultStats(DataType dt) {
	switch (dt.get()) {
		case F32 : return CellStats(); // NaN
		case F64 : return CellStats(); // NaN
		case B8  : return defaultStats1<B8 >();
		case U8  : return defaultStats1<U8 >();
		case U16 : return defaultStats1<U16>();
		case U32 : return defaultStats1<U32>();
		case U64 : return defaultStats1<U64>();
		case S8  : return defaultStats1<S8 >();
		case S16 : return defaultStats1<S16>();
		case S32 : return defaultStats1<S32>();
		case S64 : return defaultStats1<S64>();
		default  : assert(0);
	}
	return CellStats();
}

} } // namespace map::detail
