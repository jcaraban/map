/**
 * @file	DataStats.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO? use hash instead of vector? the distrubuted nodes only deal with part of the dataset
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
	type max;
	type mean;
	type min;
	type std;
	
	std::vector<type> maxb;
	std::vector<type> meanb;
	std::vector<type> minb;
	std::vector<type> stdb;
	
	DataStats();
};

inline DataStats::DataStats()
	: active(false)
	, max()
	, mean()
	, min()
	, std()
	, maxb()
	, meanb()
	, minb()
	, stdb()
{ }

} } // namespace map::detail

#endif
