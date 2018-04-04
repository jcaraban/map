/**
 * @file    diver.cpp
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Sample 1b
 * Local operations
 *
 * Calculates the variety, majority and minority of 4 rasters.
 */

#include "../map.hpp"
using namespace map;
#include <iostream>
#include <string>
#include <cstdio>


int main(int argc, char **argv) {
    Raster dem[4], out;
    std::string in_file_path[4], out_file_path;

    //// Arguments
    assert(argc > 5);
    for (int i=0; i<4; i++)
        in_file_path[i] = std::string(argv[i+1]);
    out_file_path = std::string(argv[5]);

    //// Configuration

    setupDevices("",DEV_GPU,"");
    //setupDevices("Intel",DEV_CPU,"");

    //// Computation
    
    for (int i=0; i<4; i++)
        dem[i] = read(in_file_path[i]);
    out = mean(dem[0],dem[1],dem[2],dem[3]) + dem[0]*0;
    
    write(out, out_file_path);
}
