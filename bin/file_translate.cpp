#include "../map.hpp"
using namespace map;
#include <string>


int main(int argc, char **argv) {
	Raster din, dout;
	std::string in_file_path, out_file_path;

	//// Arguments

	assert(argc > 2);
	in_file_path = std::string(argv[1]);
	out_file_path = std::string(argv[2]);

	//// Pre-Computation

	setupDevices("Intel",DEV_CPU,"");
	
	//// Computation (casting might happen!)

	din = read(in_file_path);
	info(din);
	dout = astype(din,din.datatype());
	write(dout,out_file_path);
}