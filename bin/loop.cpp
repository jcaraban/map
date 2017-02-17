/**
 * @file    loop.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 6
 * Symbolic loop
 *
 * Fills the pits in a DEM.
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>


float inf  = float("inf");
int nbh0 = {1,1,0,-1,-1,-1,0,1}
int nbh1 = {0,1,1,1,0,-1,-1,-1}

Raster border(Raster raster) {
	auto ds = raster.datasize();
	auto idx0 = index(raster,D1);
	auto idx1 = index(raster,D2);
	auto brd0 = (idx0 == 0) || (idx0 == ds[0]-1);
	auto brd1 = (idx1 == 0) || (idx1 == ds[1]-1);
	return brd0 || brd1
}

Raster pitFill(Raster dem, Raster stream) {
	auto cond = stream + border(dem);
	auto elev = con(cond, dem, +inf);
	//auto nbh  = neighborhood([3,3]);
	auto nbh = {nbh0[0],nbh1[0]};

	loop_start();
	elev == inf;
	loop_body();
	elev = lmin( max( min( elev(nbh) , elev ) , dem ) )
	loop_end();

	return elev
}

int main(int argc, char **argv) {
	Raster dem, stream, out;
	std::string in_file_path, out_file_path;

	//// Arguments
	assert(argc > 2);
	in_file_path = std::string(argv[1]);
	out_file_path = std::string(argv[2]);

	//// Configuration

	//detail::Runtime::getConfig().setNumRanks(1);
	//setupDevices("Intel",DEV_CPU,"");
	setupDevices("",DEV_GPU,"");

	//// Computation

	dem = read(in_file_path)
	stream = zeros_like(dem)
	out = pitFill(dem,stream)
	write(out, out_file_path)
}