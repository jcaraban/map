/**
 * @file	urban_mc.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 4d
 *
 * Cellular automata for urban growth simulation + Monte Carlo strategy to compose a probability map
 */

#include "../map.hpp"
using namespace map;


int main(int argc, char **argv)
{	
	//// Arguments

	assert(argc > 8);

	//// Configuration

	setupDevices("",DEV_GPU,"");
	//setupDevices("Intel",DEV_CPU,"");

	//// Urban
	
	auto urban = [&](int mc, Raster prob)
	{
		//// Inputs

		const float a = 6.4640f;	// Constant coefficient
		const float b1 = 43.5404f;	// Elevation coefficient
		const float b2 = 1.9150f;	// Slope coefficient
		const float b3 = 41.3441f;	// Distance to city centers coefficients
		const float b4 = 12.5878f;	// Distance to transportations coefficient
		std::vector<VariantType> b5 = {0.0f,0.0f,0.0f,-9.8655f,-8.7469f,-9.2688f,-8.0321f,-9.1693f,-8.9420f,-9.4500f};
		// Land use {null,water,urban,barren,forest,shrubland,woody,herbaceous,cultivated,wetlad}
		float d = 5; // dispersion parameter
		int q = 16000; // max # of cells to be converted per it (user defined / from historic data)

		auto x1 = read(argv[1]); // Elevation layer
		auto x2 = read(argv[2]); // Slope layer
		auto x3 = read(argv[3]); // Distance to centers layer
		auto x4 = read(argv[4]); // Distance to transportations layers
		auto x5 = read(argv[5]); // Land use layers
		auto e  = read(argv[6]); // exclusion layer (user defined)
		auto s  = read(argv[7]); // initial urban state

		x1 = x1 + zeros()*prob;
		x2 = x2 + zeros()*prob;
		x3 = x3 + zeros()*prob;
		x4 = x4 + zeros()*prob;
		x5 = x5 + zeros()*prob;
		e  = e  + zeros()*prob;
		s  = s  + zeros()*prob;
		
		const int N = (argc > 9) ? atoi(argv[9]) : 1 ;
		
		//// Computation
		
		for (int it=0; it<N; it++)
		{
			// Rule 1
			auto z = a + x1*b1 + x2*b2 + x3*b3 + x4*b4 + pick(x5,b5);
			auto pg = 1 / (1 + exp(z));

			// Rule 2
			Array<uint8_t> mask = {1, 1, 1, 1, 0, 1, 1, 1, 1 };
			Mask S = Mask({3,3}, mask);
			auto pc = barrier(pg * !e) * !s * convolve(s,S) / (3*3-1);

			// Rule 3
			auto pd = pc * exp(-d * (1 - pc / zonalMax(pc)));

			// Rule 4
			auto ps = q * pd / zonalSum(pd);

			// Random selection
			auto seed = zeros_like(s) + ((it + N*mc) * q + d);
			s = s || ps > rand(seed); // ps > rand() becomes urban
		}

		return s;
	};

	////  Monte Carlo

	auto prob = zeros(); // probability map of turning urban
	const int M = (argc > 10) ? atoi(argv[10]) : 1 ;

	for (int mc=0; mc<M; mc++)
		prob = barrier(prob + urban(mc,prob));
	prob = prob / (float)M;

	write(prob,argv[8]);
}
