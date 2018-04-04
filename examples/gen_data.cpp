/**
 * @file    gen_data.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Generates a syhnthetic DEM with wave-like mountains
 *
 * TODO: think about multi-layer support. That would allow 'dif = center - index(out)'
 */


#include "../map.hpp"
#include <iostream>
#include <string>
using namespace map;

#define PI 3.141593f


int main(int argc, char **argv) {
	Raster dout;
	std::string file_path;
	DataSize data_size;
	BlockSize block_size = {512,512};
	BlockSize group_size = {16,16};
	int steep = false;

	//// Arguments

	assert(argc > 3);

	file_path = std::string(argv[1]);
	int sx = atoi(argv[2]);
	int sy = atoi(argv[3]);
	data_size = {sx,sy};
	if (argc > 4) {
		int sb = atoi(argv[4]);
		block_size = {sb,sb};
	}
	if (argc > 5) {
		steep = atoi(argv[5]);
	}

	assert(data_size[0] % block_size[0] == 0 && data_size[1] % block_size[1] == 0);

	//// Pre-Computation

	detail::Runtime::getConfig().setNumRanks(1);
	setupDevices("",DEV_GPU,"");
	//setupDevices("Intel",DEV_CPU,"");

	dout = zeros(data_size,F32,ROW+BLK,block_size,group_size);
	info(dout);

	//// Computation
	
	int center_x = data_size[0] / 2;
	int center_y = data_size[1] / 2;
	float max_dif = (float) sqrt((double)center_x*center_x + (double)center_y*center_y);

	auto dif_x = center_x - index(dout,D1);
	auto dif_y = center_y - index(dout,D2);
	auto phi = atan2(1.0f*dif_y,1.0f*dif_x) + PI;
	auto dist = sqrt(1.0f*dif_x*dif_x + dif_y*dif_y) / max_dif;
	auto r = (cos(phi*16 + dist*PI*2) + 1) / 2;
	auto value = (cos(dist*PI*16)+1)/2 * dist * r;
	value = value * 0.9f + dist * 0.1f; // @

	value = con(steep, floor(value*10)/10, value);
	dout = value;

	//dout.stats();
	
	//// Post-Computation
	
	write(dout,file_path);
}
