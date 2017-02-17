/**
 * @file    wsum.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 1
 * Local operations
 *
 * Calculates the weighted summation of a 4 rasters.
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>


int main(int argc, char **argv) {
	Raster dem[4], out;
	std::string in_file_path[4], out_file_path;

	//// Arguments
	assert(argc > 5);
	for (int i=0; i<4; i++)
		in_file_path[i] = std::string(argv[i+1]);
	out_file_path = std::string(argv[5]);

	//// Configuration

	//detail::Runtime::getConfig().setNumRanks(1);
	//setupDevices("Intel",DEV_CPU,"");
	setupDevices("",DEV_GPU,"");

	//// Computation
	
	for (int i=0; i<4; i++)
		dem[i] = read(in_file_path[i]);
	out = dem[0]*0.1f + dem[1]*0.2f + dem[2]*0.3f + dem[3]*0.4f;
	
	write(out, out_file_path);
}
