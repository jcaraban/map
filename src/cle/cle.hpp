/**
 * @file	cle.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * OpenCL: includes, defines, typedef, options and variables
 *
 * TODO: should DeviceType follow cl_device_type or rather keep the already used enum style ( NONE_DEVICE, ... , N_DEVICE )
 */

#ifndef MAP_CLE_HPP_
#define MAP_CLE_HPP_

#include "OclEnv.hpp"


namespace map { namespace detail {

enum MemFlag
{
	MEM_READ_ONLY = CL_MEM_READ_ONLY,
	MEM_WRITE_ONLY = CL_MEM_WRITE_ONLY,
	MEM_READ_WRITE = CL_MEM_READ_WRITE
};

enum MapFlags
{
	MAP_READ = CL_MAP_READ,
	MAP_WRITE = CL_MAP_WRITE,
	MAP_WRITE_TRUNC = CL_MAP_WRITE_INVALIDATE_REGION
};

enum DeviceType
{
	DEV_DEF = CL_DEVICE_TYPE_DEFAULT,
	DEV_CPU = CL_DEVICE_TYPE_CPU,
	DEV_GPU = CL_DEVICE_TYPE_GPU,
	DEV_ACC = CL_DEVICE_TYPE_ACCELERATOR,
	DEV_CUS = CL_DEVICE_TYPE_CUSTOM,
	DEV_ALL = CL_DEVICE_TYPE_ALL
};

//enum DeviceType { NONE_DEVICE, CPU, GPU, PHI, N_DEVICE_TYPE };

inline DeviceType cledev2devtype(cle::Device dev) {
	cl_device_type type = *(cl_device_type*) dev.get(CL_DEVICE_TYPE);
	switch (type) {
		case CL_DEVICE_TYPE_CPU:		 return DEV_CPU;
		case CL_DEVICE_TYPE_GPU:		 return DEV_GPU;
		case CL_DEVICE_TYPE_ACCELERATOR: return DEV_ACC;
		default: assert(0);
	}
}

} } // namespace map::detail

#endif
