/**
 * @file    stats.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 2
 * Zonal operations
 *
 * Calculates the statistics of a map and perform few local operations with them.
 *
 * TODO: add the calculation of the geometric mean (would balance script more toward comptutation)
 * TODO: the conversation of Array should be substituted with a dynamic function .toU64()
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>


int main(int argc, char **argv) {
	std::string in_file_path;

	//// Arguments
	assert(argc > 1);
	in_file_path = std::string(argv[1]);

	//// Configuration

	//detail::Runtime::getConfig().setNumRanks(1);
	//setupDevices("Intel",DEV_CPU,"");
	setupDevices("",DEV_GPU,"");

	//// Computation
	
	auto dem = read(in_file_path);
	auto N = prod(Array4<uint64_t>(dem.datasize()));

	auto maxv = zonalMax(dem);
	auto minv = zonalMin(dem);
	auto rang = maxv - minv;

	auto mean = zonalSum(dem) / N;
	auto geom = pow(zonalProd(dem),1.0/N);
	auto harm = N / zonalSum(1.0/dem);
	auto quam = sqrt(zonalSum(pow(dem,2))/N);
	auto cubm = cbrt(zonalSum(pow(dem,3))/N);

	auto dev  = (dem - mean) * (dem - mean);
	auto var  = zonalSum(dev) / N;
	auto std  = sqrt(var);

	auto norm = (dev - minv) / rang;
	auto nstd = sqrt(zonalSum(norm) / N);

	//std::cerr << value(nstd) << std::endl;
	eval({maxv,minv,mean,geom,harm,quam,cubm,std,nstd});
	std::cerr << value(maxv) << " " << value(minv) << " " << value(rang) << " " << value(mean) << " " << value(geom) << " " <<
		value(harm) << " " << value(quam) << " " << value(cubm) << " " << value(std) << " " << value(nstd) << std::endl;
}
