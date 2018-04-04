/**
 * @file	life.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 4
 * Piped Focal operations
 *
 * Conway game of life
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <unistd.h>


Raster life(Raster dem) {
	Array<uint8_t> h = {1, 1, 1, 1, 0, 1, 1, 1, 1 };
	Mask S = Mask({3,3}, h);
	return convolve(dem,S);
}

int main(int argc, char **argv) {
	Raster state, nbh, random, x, y;
	DataSize data_size;
	BlockSize block_size = {512,512};
	GroupSize group_size = {16,16};
	int r, N = 16;

	//// Arguments

	assert(argc > 3);

	std::string file = argv[1];
	data_size = {atoi(argv[2]),atoi(argv[3])};
	if (argc > 4) block_size = {atoi(argv[4]),atoi(argv[4])};
	if (argc > 5) N = atoi(argv[5]);

	assert(data_size[0] % block_size[0] == 0 && data_size[1] % block_size[1] == 0);

	//// Configuration

	//detail::Runtime::getConfig().setNumRanks(1);
	//setupDevices("Intel",DEV_CPU,"");
	setupDevices("",DEV_GPU,"");

	//// Computation
	
	state = rand(N,data_size,U8,ROW+BLK,block_size,group_size) > 128;
	//state = rand(N,data_size,U8,ROW+BLK,block_size) * 0;

	r = data_size[0] / 2;
	x = r - index(state,D1);
	y = r - index(state,D2);
	state = con(x*x+y*y<r*r/2,state,zeros_like(state));
	state = stats(state); // @
	
	for (int it=0; it<N; it++) {
		nbh = life(state);
		state = (nbh == 3) + (nbh == 2) * state;

		if (it % 5 == 4)
			state = stats(state); // @
	}

	write(state,file);
}
