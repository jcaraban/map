/**
 * @file	DataStats.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: use hash instead of vector? the distrubuted nodes only deal with part of the dataset
 * TODO: shall the vector hold BlockStats rather than Unions? more generic, more posibilities!
 */

#ifndef MAP_FILE_DATASTATS_HPP_
#define MAP_FILE_DATASTATS_HPP_

#include <vector>


namespace map { namespace detail {

/*
 * @class DataStats
 */
struct DataStats {
	//typedef Ctype<F64> type;
	typedef VariantUnion type;
	
	bool active;
	type min;
	type max;
	type mean;
	type std;
	
	std::vector<type> minb;
	std::vector<type> maxb;
	std::vector<type> meanb;
	std::vector<type> stdb;
	
	DataStats();
};

inline DataStats::DataStats()
	: active(false)
	, min()
	, max()
	, mean()
	, std()
	, minb()
	, maxb()
	, meanb()
	, stdb()
{ }

} } // namespace map::detail

#endif
