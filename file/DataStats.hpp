/**
 * @file	DataStats.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: use hash instead of vector? the distrubuted nodes only deal with part of the dataset
 * TODO: shall the vector hold BlockStats rather than Unions? more generic, more posibilities!
 *
 * TODO: rethink the whole class hierarchy, maybe add SingleStats / MultiStats ?
 * TODO: promote 'active' to a Enum-Class modeling { NONE_KNOWN, KNOWN_RANGE, KNOWN_CENTROID, KNOWN_VALUE }
 */

#ifndef MAP_FILE_DATASTATS_HPP_
#define MAP_FILE_DATASTATS_HPP_

#include "../util/util.hpp"
#include <vector>


namespace map { namespace detail {

/*
 * @class CellStats
 */
struct CellStats {
	//typedef VariantUnion type;

	bool active;
	DataType data_type;

	VariantType min;
	VariantType max;
	VariantType mean;
	VariantType std;

	CellStats();
};

/*
 * @class DataStats
 */
struct DataStats {
	typedef VariantUnion type;
	
	bool active;
	DataType data_type;
	NumBlock num_block;

	VariantType min;
	VariantType max;
	VariantType mean;
	VariantType std;
	
	std::vector<VariantUnion> minb;
	std::vector<VariantUnion> maxb;
	std::vector<VariantUnion> meanb;
	std::vector<VariantUnion> stdb;
	
	DataStats();
	CellStats get(Coord coord) const;
	void set(Coord coord, CellStats stats);
};

/*
 * @class BlockStats
 */
struct BlockStats {
	//typedef VariantUnion type;
	
	DataType data_type;
	NumGroup num_group;
	bool active;
	
	VariantType min;
	VariantType max;
	VariantType mean;
	VariantType std;
	
	std::vector<VariantUnion> ming;
	std::vector<VariantUnion> maxg;
	std::vector<VariantUnion> meang;
	std::vector<VariantUnion> stdg;

	BlockStats();
	bool operator==(const BlockStats &other);
	operator CellStats() const;
};

CellStats defaultStats(DataType dt);
template <DataTypeEnum T> CellStats defaultStats1();

} } // namespace map::detail

#endif

#include "DataStats.tpl"
