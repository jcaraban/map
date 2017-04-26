/**
 * @file	main.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Test program...
 */

#include "../map.hpp"
using namespace map;
#include <iostream>


int main(int argc, char **argv) {
	
	std::cout << " Is Variant standard-layout? " << std::is_standard_layout<VariantType>::value << std::endl;
	std::cout << " Is DataType standard-layout? " << std::is_standard_layout<DataType>::value << std::endl;
	//std::cout << " Is ReductionType standard-layout? " << std::is_standard_layout<ReductionType>::value << std::endl;
	std::cout << " Is Union standard-layout? " << std::is_standard_layout<map::detail::VariantUnion>::value << std::endl;
	std::cout << " Is DataType standard-layout? " << std::is_standard_layout<map::detail::DataType>::value << std::endl;
	std::cout << " Is BlockaSize standard-layout? " << std::is_standard_layout<map::detail::BlockSize>::value << std::endl;
	std::cout << "cl_mem " << sizeof(cl_mem) << std::endl;
	
	setupDevices("Intel",DEV_CPU,"");
	std::cout << value( (Raster(1) + Raster(3)) * Raster(4) ) << std::endl;

	setupDevices("",DEV_GPU,"");
	std::cout << value( Raster(1) + Raster(2) + Raster(1) * Raster(4) ) << std::endl;

	setupDevices("",DEV_GPU,"");
	std::cout << value( Raster(1) * Raster(4) + Raster(2) ) << std::endl;
}
