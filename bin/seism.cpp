/**
 * @file    seism.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 4b
 *
 * 2D simulation of a seism
 */

#include "../map.hpp"
using namespace map;
#include <string>


int main(int argc, char **argv) {
	Raster S; //!< Horizontal stress
	Raster T; //!< Vertical stress
	Raster V; //!< Velocity at each grid point
	Raster D; //!< Damping coefficients
	Raster M; //!< Coefficient related to modulus
	Raster L; //!< Coefficient related to lightness
	int pcount; //!< Pulse counter
	int ptime; //!< Pulse time
	Coord pc; //!< Pulse coordinate

	//// Arguments

	assert(argc > 3);

	int N = 16;
	std::string file = argv[1];
	DataSize ds = {atoi(argv[2]),atoi(argv[3])};
	BlockSize bs = {512,512};

	if (argc > 4)
		bs = {atoi(argv[4]),atoi(argv[4])};
	if (argc > 5)
		N = atoi(argv[5]);

	assert(ds[0] % bs[0] == 0 && ds[1] % bs[1] == 0);

	//// Configuration

	setupDevices("",DEV_GPU,"");
	//setupDevices("Intel",DEV_CPU,"");

	//// Initialization
	
	auto one = ones(ds,F32,ROW+BLK,bs);

	S = V = T = one * 1.0E-6f;
	D = one;
	/*
	float d = 1.0;
	for (int k=DamperSize-1; k>0; --k) {
		d *= 1-1.0f/(DamperSize*DamperSize);
		for (int j=1; j<UniverseWidth-1; ++j) {
			D[{k,j}] *= d;
			D[{UniverseHeight-1-k,j}] *= d;
		}
		for (int i=1; i<UniverseHeight-1; ++i) {
			D[{i,k}] *= d;
			D[{i,UniverseWidth-1-k}] *= d;
		}
	}
	*/
	auto t = index(one,D2) / (float)ds[1];
	auto x = (index(one,D1) - ds[0]/2.0f) / (ds[0]/2.0f);
	auto c1 = t < 0.3f;
	auto c2 = abs(t-0.7f+0.2f*exp(-8*x*x)+0.025f*x) <= 0.1f;
	M = con(c1,0.125f,con(c2,0.5f,0.3f));
	L = con(c1,0.125f,con(c2,0.6f,0.4f));

	pcount = ptime = 8;
    pc = {ds[0]/3,ds[1]/4};

	//// Computation

	for (int it=0; it<N; it++) {
		// Update pulse
	    if (pcount > 0) {
			float t = (pcount-ptime)/2.0f * 0.05f;
	        V[pc] += 64*sqrt(M[pc])*(float)exp(-t*t);
	        pcount--;
	    }
		// Update stress
		S = S + M * (V({1,0})-V);
		T = S + M * (V({0,1})-V);
		// Update velocity
		V = D * (V + L * (S - S({-1,0}) + T - T({0,-1}) ));
	}

	write(V, file);
}
