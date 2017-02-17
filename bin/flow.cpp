/**
 * @file    flow.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 6
 * Spreading operations
 *
 * Calculates the flow accumulation in a DEM for a constant water body of 1xCell
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>


int main(int argc, char **argv) {
    Raster dem, dir, pit, water, acu;
    std::string in_file_path, out_file_path;
    int N = 4;

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
    dir = flowDir(dem);
    water = ones_like(dem,U32) + zeros_like(dem,U32);
    acu = flowAccumulation(water,dir) + zeros_like(dem,U32);
    write(acu, out_file_path);
}
