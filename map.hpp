/** 
 * @file    map.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * 1. Includes all headers with items that should be visible to the user
 * 2. Makes items visible with the 'using' keyword
 */

#ifndef MAP_HPP_
#define MAP_HPP_

#include "src/frontend/Raster.hpp"
#include "src/util/util.hpp"
#include "src/Runtime.hpp"
 

namespace map {
	// Raster.hpp
	using detail::Raster;
	using detail::Mask;
	using detail::eval;
	using detail::value;
	using detail::info;
	using detail::stats;
	using detail::read;
	using detail::write;
	using detail::zeros;
	using detail::ones;
	using detail::index;
	using detail::convolve;
	using detail::viewshed;
	using detail::operator+;
	using detail::operator-;
	using detail::operator*;
	using detail::operator/;
	using detail::operator!;
	using detail::operator~;
	using detail::operator%;
	using detail::operator==;
	using detail::operator!=;
	using detail::operator<;
	using detail::operator>;
	using detail::operator<=;
	using detail::operator>=;
	using detail::operator&&;
	using detail::operator||;
	using detail::operator&;
	using detail::operator|;
	using detail::operator^;
	using detail::operator<<;
	using detail::operator>>;
	using detail::sin;
	using detail::cos;
	using detail::atan;
	using detail::abs;
	using detail::sqrt;
	using detail::max;
	using detail::min;
	using detail::atan2;
	using detail::sum;
	using detail::prod;

	// Array.hpp
	using detail::Array;
	using detail::Array4;
	using detail::DataSize;
	using detail::BlockSize;
	using detail::GroupSize;
	using detail::NumBlock;
	using detail::NumGroup;
	using detail::Coord;
	using detail::operator+;
	using detail::operator-;
	using detail::operator*;
	using detail::operator/;
	using detail::operator!;
	using detail::operator~;
	using detail::operator%;
	using detail::operator==;
	using detail::operator!=;
	using detail::operator<;
	using detail::operator>;
	using detail::operator<=;
	using detail::operator>=;
	using detail::operator&&;
	using detail::operator||;
	using detail::operator&;
	using detail::operator|;
	using detail::operator^;
	using detail::operator<<;
	using detail::operator>>;
	using detail::all;
	using detail::any;
	using detail::sum;
	using detail::prod;
	using detail::proj;
	using detail::in_range;

	// Defines.hpp
	using detail::StreamDir;
		using detail::NONE_STREAMDIR;
		using detail::IN;
		using detail::OUT;
		using detail::IO;
	using detail::DataType;
		using detail::NONE_DATATYPE;
		using detail::F32;
		using detail::F64;
		using detail::B8;
		using detail::U8;
		using detail::U16;
		using detail::U32;
		using detail::U64;
		using detail::S8;
		using detail::S16;
		using detail::S32;
		using detail::S64;
	using detail::NumDim;
		using detail::NONE_NUMDIM;
		using detail::TIME;
		using detail::D0;
		using detail::D1;
		using detail::D2;
		using detail::D3;
	using detail::MemOrder;
		using detail::NONE_MEMORDER;
		using detail::BLK;
		using detail::ROW;
		using detail::COL;
		using detail::SFC;
	using detail::DeviceType;
		using detail::DEV_DEF;
		using detail::DEV_CPU;
		using detail::DEV_GPU;
		using detail::DEV_ACC;
		using detail::DEV_ALL;
	using detail::VariantType;
	using detail::operator+;

	// Runtime.hpp
	using detail::eval;
	//void eval() { map::detail::eval(); }
	//map::detail::Config& getConfig() { return map::detail::getConfig(); }
	//cle::OclEnv& getOclEnv() { return detail::Runtime::getOclEnv(); }
	using detail::setupDevices;

	// viewshed.hpp
}

#endif
