/**
 * @file    gis1.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 5
 * Radiating operations
 *
 * Calculates the viewshed of a map for a given observer coordinates and height
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>


int main(int argc, char **argv) {
	std::string in_file_path, out_file_path;
	int obs_x, obs_y;
	float obs_h;

	//// Arguments

	assert(argc > 5);
	in_file_path = std::string(argv[1]);
	out_file_path = std::string(argv[2]);
	obs_x = atoi(argv[3]);
	obs_y = atoi(argv[4]);
	obs_h = atof(argv[5]);

	//// Configuration

	//detail::Runtime::getConfig().setNumRanks(1);
	//setupDevices("Intel",DEV_CPU,"");
	setupDevices("",DEV_GPU,"");

	//// Computation
	
	Array<float> f = { 1.0/16, 1.0/8, 1.0/16, 1.0/8, 1.0/4, 1.0/8, 1.0/16, 1.0/8, 1.0/16 };
	Mask F = Mask({3,3}, f);

	auto dem = read(in_file_path);
	auto minv = zonalMin(dem);
	auto maxv = zonalMax(dem);
	auto norm = (dem - minv) / (maxv - minv);
	auto view = viewshed(norm, {obs_x,obs_y}, obs_h);
	auto soft = convolve(view, F);
	auto out = con(soft < 0.1f, dem, zeros_like(dem));
	write(out, out_file_path);
}
