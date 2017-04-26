/**
 * @file	Direction.hpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: study how to make variables constants but still keep the assignment operator
 */

#ifndef MAP_UTIL_DIRECTION_HPP
#define MAP_UTIL_DIRECTION_HPP

#include "../util/Array4.hpp"
#include <string>


namespace map { namespace detail {

struct North { };
struct East { };
struct South { };
struct West { };

struct Direction {
  bool north, east, south, west;
  Direction() : north(0), east(0), south(0), west(0) { }
  Direction(North n) : north(1), east(0), south(0), west(0) { }
  Direction(East e) : north(0), east(1), south(0), west(0) { }
  Direction(South s) : north(0), east(0), south(1), west(0) { }
  Direction(West w) : north(0), east(0), south(0), west(1) { }
  Direction(bool n, bool e, bool s, bool w) : north(n), east(e), south(s), west(w) { }
  Direction(const Direction &d) : north(d.north), east(d.east), south(d.south), west(d.west) { }

  std::string north2str();
  std::string east2str();
  std::string south2str();
  std::string west2str();
  std::string toString();

  bool isNone();
  bool isSame(Direction);
  bool isAnti(Direction);
  Array4<bool> isForward();
  Array4<bool> isBackward();
  Direction rotateRight();
  Direction rotateLeft();
  Direction reverse();
  Array4<int> unitVec();
  Array4<int> dimVec();
};

inline std::string Direction::north2str() { return north ? "true" : "false"; }
inline std::string Direction::east2str() { return east ? "true" : "false"; }
inline std::string Direction::south2str() { return south ? "true" : "false"; }
inline std::string Direction::west2str() { return west ? "true" : "false"; }
inline std::string Direction::toString() { return north ? "North" : east ? "East" : south ? "South" : west ? "West" : ""; }

inline bool Direction::isNone() { return !north && !east && !south && !west; }
inline bool Direction::isSame(Direction d) { return north==d.north && east==d.east && south==d.south && west==d.west; }
inline bool Direction::isAnti(Direction d) { return (north && d.south) || (east && d.west) || (south && d.north) || (west && d.east); }
inline Array4<bool> Direction::isForward() { return {east,south}; }
inline Array4<bool> Direction::isBackward() { return {west,north}; }
inline Direction Direction::rotateRight() { return Direction(west,north,east,south); }
inline Direction Direction::rotateLeft() { return Direction(east,south,west,north); }
inline Direction Direction::reverse() { return Direction(south,west,north,east); }
inline Array4<int> Direction::unitVec() { return {east - west, south - north}; }
inline Array4<int> Direction::dimVec() { return {east || west, south || north}; }

/****************
   Radial Cases
 ****************/

enum RadialCase { NORTH_EAST, EAST_NORTH, EAST_SOUTH, SOUTH_EAST, SOUTH_WEST, WEST_SOUTH, WEST_NORTH, NORTH_WEST, N_RADIAL_CASE };

inline void radia2dir(RadialCase rcase, Direction &first, Direction &second) {
	if (rcase == NORTH_EAST) {
		first = Direction( North() );
		second = Direction( East() );
	} else if (rcase == EAST_NORTH) {
		first = Direction( East() );
		second = Direction( North() );
	} else if (rcase == EAST_SOUTH) {
		first = Direction( East() );
		second = Direction( South() );
	} else if (rcase == SOUTH_EAST) {
		first = Direction( South() );
		second = Direction( East() );
	} else if (rcase == SOUTH_WEST) {
		first = Direction( South() );
		second = Direction( West() );
	} else if (rcase == WEST_SOUTH) {
		first = Direction( West() );
		second = Direction( South() );
	} else if (rcase == WEST_NORTH) {
		first = Direction( West() );
		second = Direction( North() );
	} else if (rcase == NORTH_WEST) {
		first = Direction( North() );
		second = Direction( West() );
	} // else, first / second unchanged
}

inline void coord2dir(int x, int y, Direction &first, Direction &second) {
	/****/ if (x == 0 && y < 0) {
		first = Direction( North() );
		second = Direction();
	} else if (x > 0 && y == 0) {
		first = Direction( East() );
		second = Direction();
	} else if ( x == 0 && y > 0) {
		first = Direction( South() );
		second = Direction();
	} else if ( x < 0 && y == 0) {
		first = Direction( West() );
		second = Direction();
	} else if (x > 0 && y < 0 && x <= -y) { // <=
		first = Direction( North() );
		second = Direction( East() );
	} else if (x > 0 && y < 0 && x > -y) {
		first = Direction( East() );
		second = Direction( North() );
	} else if (x > 0 && y > 0 && x >= y) { // >=
		first = Direction( East() );
		second = Direction( South() );
	} else if (x > 0 && y > 0 && x < y) {
		first = Direction( South() );
		second = Direction( East() );
	} else if (x < 0 && y > 0 && -x <= y) { // <=
		first = Direction( South() );
		second = Direction( West() );
	} else if (x < 0 && y > 0 && -x > y) {
		first = Direction( West() );
		second = Direction( South() );
	} else if (x < 0 && y < 0 && -x >= -y) { // >=
		first = Direction( West() );
		second = Direction( North() );
	} else if (x < 0 && y < 0 && -x < -y) {
		first = Direction( North() );
		second = Direction( West() );
	} else if (x == 0 && y == 0) {
		first = Direction();
		second = Direction();
	} else
		assert(0);
}

inline RadialCase dir2radia(Direction first, Direction second) {
	/**/ if (first.north && second.east)
		return NORTH_EAST;
	else if (first.east && second.north)
		return EAST_NORTH;
	else if (first.east && second.south)
		return EAST_SOUTH;
	else if (first.south && second.east)
		return SOUTH_EAST;
	else if (first.south && second.west)
		return SOUTH_WEST;
	else if (first.west && second.south)
		return WEST_SOUTH;
	else if (first.west && second.north)
		return WEST_NORTH;
	else if (first.north && second.west)
		return NORTH_WEST;
	else
		assert(0);
}

inline std::string radia2str(RadialCase rcase) {
	/****/ if (rcase == NORTH_EAST) {
		return "NorthEast";
	} else if (rcase == EAST_NORTH) {
		return "EastNorth";
	} else if (rcase == EAST_SOUTH) {
		return "EastSouth";
	} else if (rcase == SOUTH_EAST) {
		return "SouthEast";
	} else if (rcase == SOUTH_WEST) {
		return "SouthWest";
	} else if (rcase == WEST_SOUTH) {
		return "WestSouth";
	} else if (rcase == WEST_NORTH) {
		return "WestNorth";
	} else if (rcase == NORTH_WEST) {
		return "NorthWest";
	} else {
		assert(0);
	}
}

inline RadialCase str2radia(std::string str) {
	/****/ if (str.compare("NorthEast") == 0) {
		return NORTH_EAST;
	} else if (str.compare("EastNorth") == 0) {
		return EAST_NORTH;
	} else if (str.compare("EastSouth") == 0) {
		return EAST_SOUTH;
	} else if (str.compare("SouthEast") == 0) {
		return SOUTH_EAST;
	} else if (str.compare("SouthWest") == 0) {
		return SOUTH_WEST;
	} else if (str.compare("WestSouth") == 0) {
		return WEST_SOUTH;
	} else if (str.compare("WestNorth") == 0) {
		return WEST_NORTH;
	} else if (str.compare("NorthWest") == 0) {
		return NORTH_WEST;
	} else {
		assert(0);
	}
}

} } // namespace map::detail

#endif
