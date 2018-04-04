/**
 * @file	urban.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Cellular automata for olive fly
 */

#include "../map.hpp"
using namespace map;


int main(int argc, char **argv)
{	
	//// Arguments

	assert(argc > 5);

	//// Configuration

	//setupDevices("",DEV_GPU,"");
	setupDevices("Intel",DEV_CPU,"");

	//// Inputs

	auto f = read(argv[1]); // number of flies
	auto o = read(argv[2]); // olive grove
	auto t = read(argv[3]); // avg. annual temperature
	auto r = read(argv[4]); // avg. annual rain
	auto w = read(argv[5]); // rivers network
	Raster s, br, dr;

	auto N  = (argc > 7) ? atoi(argv[7]) : 1 ;

	std::vector<VariantType> tT = {0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f,1.0f};
	std::vector<VariantType> rT = {0.5f,0.45f,0.4f,0.35f,0.3f,0.25f,0.2f,0.15f,0.1f,0.05f,0.0f};

	auto spread = [](Raster f) {
		Array<int> h = { 2, 3, 3, 3, 2,
						 3, 2, 2, 2, 3,
						 3, 2, 1, 2, 3,
						 3, 2, 2, 2, 3,
						 2, 3, 3, 3, 2 };
		Mask H = Mask({5,5}, h);
		return convolve(f,H) / 61.0f;
	};

	// Main Loop

	for (int j=0; j<N; j++) {
		s = 0.5f*w + pick(r,rT);
		f = spread(f);
		br = o * s * (1-f)*f*4;
		f = f + 0.1f*br * rand(f+j);
		dr = pick(t,tT) * f;
		f = f - 0.1f*dr * rand(f+j);
	}

	f = stats(f);
	write(f,argv[6]);
}
