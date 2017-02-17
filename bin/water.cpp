/**
 * @file	urban.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Cellular automata for water flow
 */

#include "../map.hpp"
using namespace map;


int main(int argc, char **argv)
{	
	//// Arguments

	assert(argc > 5);

	//// Configuration

	setupDevices("",DEV_GPU,"");
	//setupDevices("Intel",DEV_CPU,"");

	//// Inputs

	auto h = read(argv[1]); // digital elevation layer
	auto w = read(argv[2]); // water level layer
	auto i = read(argv[3]); // inlets inflow layer
	auto o = read(argv[4]); // outlets outflow layer
	Raster l;

	auto N = (argc > 6) ? atoi(argv[6]) : 1 ;

	auto swap = [&](std::vector<Raster> &x, int i, int j) {
		auto aux = max(x[i],x[j]);
		x[i] = min(x[i],x[j]);
		x[j] = aux;
	};

	auto netsort5 = [&](std::vector<Raster> &x) {
		swap(x,0,1); swap(x,2,3); swap(x,0,2);
		swap(x,3,4); swap(x,0,3); swap(x,1,3);
		swap(x,2,4); swap(x,1,4); swap(x,1,2);
	};

	auto avglevel = [&](Raster w, Raster h, std::vector<Raster> &x) {
		netsort5(x);     // in ascending order
		auto e = 0.001f; // epsilon
		auto s = w+x[0]; // sum variable
		auto c = ones(); // count variable
		for (int i=1; i<5; i++) {
			auto b = (s-e >= x[i]*i);
			s = s + b*x[i];
			c = c + b;
		}
		return s / c;
	};

	auto gather = [&](Raster w, Raster h) {
		std::vector<Raster> x(5); // list of neighbors
		x[0] = h; // central cell height
		x[1] = (h+w)({0,-1});
		x[2] = (h+w)({-1,0});
		x[3] = (h+w)({+1,0});
		x[4] = (h+w)({0,+1});
		return avglevel(w,h,x);
	};

	auto distrib = [&](Raster w, Raster h, Raster l) {
		auto wh = w+h;    // prev water level
		auto c = zeros(); // change in water
		c = c + max(0.0f, l({0,-1}) - wh);
		c = c + max(0.0f, l({-1,0}) - wh);
		c = c + max(0.0f, l({+1,0}) - wh);
		c = c + max(0.0f, l({0,+1}) - wh);
		c = c + max(h, l) - wh;
		auto cwh = max(c + wh, h);
		return cwh - h;
	};

	auto inflow = [&](Raster w, Raster i) {
		return w + i;
	};

	auto outflow = [&](Raster w, Raster o) {
		return max(w - o, zeros());
	};

	// Main Loop

	for (int j=0; j<N; j++) {
		w = inflow(w,i);    // add water to the inlets
		l = gather(w,h);    // gather the avg. water level
		w = distrib(w,h,l); // distribute the water
		w = outflow(w,o);   // remove water from the outlets
	}

	write(w,argv[5]);
}
