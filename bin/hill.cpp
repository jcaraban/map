/**
 * @file    hill.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 3
 * Focal operations
 *
 * Calculates the hillsade of a map, for which slope and aspect are required. Then colorizes the output with a colormap
 * This is a simple program involving LOCAl and FOCAL operations, but it posses plenty of opportunities for optimization.
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>

#define PI 3.141593f


Raster hori(Raster dem, float dist) {
	Array<int> h = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	Mask H = Mask({3,3}, h);
	return convolve(dem,H) / (8 * dist);
}

Raster vert(Raster dem, float dist) {
	Array<int> v = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
	Mask V = Mask({3,3}, v);
	return convolve(dem,V) / (8 * dist);
}

Raster slope(Raster dem, float z_factor=1, float dist=1) {
	auto x = hori(dem,dist);
	auto y = vert(dem,dist);
	auto z = atan(z_factor * sqrt(x*x + y*y));
	return z;
}

Raster aspect(Raster dem, float dist=1) {
	auto x = hori(dem,dist);
	auto y = vert(dem,dist);
	auto z1 = (x!=0) * atan2(y,-x);
	z1 = z1 + (z1<0) * (PI*2);
	auto z0 = (x==0) * ((y>0)*(PI/2) + (y<0)*(PI*2-PI/2));
	return z1 + z0;
}

Raster hillshade(Raster dem, float zenith, float azimuth, float z_factor=1) {
	float zr = (90 - zenith) / 180 * PI;
	float ar = 360 - azimuth + 90;
	ar = (ar > 360) ? ar - 360 : ar;
	ar = ar / 180 * PI;
	auto hs = (( (float)cos(zr) * cos(slope(dem,z_factor)) ) +
			  ( (float)sin(zr) * sin(slope(dem,z_factor)) * cos(ar-aspect(dem)) ));
	return hs;
}


int main(int argc, char **argv) {
	Raster dem, scaled, hill, out;
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
	
	dem = read(in_file_path);
	out = hillshade(dem,45,315); //overlay(dem, hillshade(dem,45,315));
	write(out, out_file_path);
}
