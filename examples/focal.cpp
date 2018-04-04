/**
 * @file    focal.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 3b
 * Focal operations
 *
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>


int main(int argc, char **argv) {
    Raster dem, scaled, hill, out;
    std::string in_file_path, out_file_path;
    int N = 1;

    //// Arguments

    assert(argc > 2);
    in_file_path = std::string(argv[1]);
    out_file_path = std::string(argv[2]);
    if (argc > 3) N = atoi(argv[3]);

    //// Configuration

    setupDevices("",DEV_GPU,"");
    //setupDevices("Intel",DEV_CPU,"");
    //detail::Runtime::getConfig().setNumRanks(1);

    //// Computation
    
    dem = read(in_file_path);
    Array<int> m = {1, 1, 1, 1, 1, 1, 1, 1, 1 };
    Mask M({3,3},m);

    //out = (maxFocal(dem,1) - minFocal(dem,1)) + (prodFocal(dem,1) / sumFocal(dem,1));
    //out = flowDirection(dem);

    //for (int i=0; i<N; i++)
    //    dem = convolve(dem,M);
    dem = max(convolve(dem,M)) + dem;

    write(dem, out_file_path);
}
