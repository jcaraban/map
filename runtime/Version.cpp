/**
 * @file    Version.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Version.hpp"
#include "task/Task.hpp"
#include "Runtime.hpp"
#include <fstream>

namespace map { namespace detail {

Version::Version(Task *task, cle::Device dev, std::string detail)
	: task(task)
	, dev(dev)
	, detail(detail)
{
	// Filling 'dev_type'
	cl_device_type type = *(cl_device_type*) dev.get(CL_DEVICE_TYPE);
	switch (type) {
		case CL_DEVICE_TYPE_CPU:		 dev_type = DEV_CPU; break;
		case CL_DEVICE_TYPE_GPU:		 dev_type = DEV_GPU; break;
		case CL_DEVICE_TYPE_ACCELERATOR: dev_type = DEV_ACC; break;
		default: assert(0);
	}
	// Filling 'signature'
	ver_sign = task->group()->signature() + detail + std::to_string(deviceType());
}

cle::Device Version::device() const {
	return dev;
}

DeviceType Version::deviceType() const {
	return dev_type;
}

std::string Version::signature() const {
	return ver_sign;
}

const BlockSize& Version::groupsize() const {
	return group_size;
}

const NumBlock& Version::numgroup() const {
	return num_group;
}

void Version::createProgram() {
	cle::Context ctx = dev.C(0); // Devices only have 1 context
	const char *code_str = code.data();
	size_t code_length = code.size();
	cl_int err;

	cl_program cl_prg = clCreateProgramWithSource(*ctx, 1, &code_str, &code_length, &err);
	cle::clCheckError(err);
	int id = ctx.addTask(cl_prg);
	this->tsk = ctx.T(id);
}

void Version::compileProgram() {
	std::string flags;
	cl_int err;

	size_t hash = std::hash<std::string>()( signature() );
	std::string kernel_name = "krn" + std::to_string(hash);

	// Includes
	#ifdef RAND123
		#define xstr(s) str(s)
		#define str(s) #s
		flags += "-I " xstr(RAND123) " ";
		#undef str
		#undef xstr
	#endif

	// Optimizations
	bool opt = false; // only with AMD
	if (opt) {
		flags += "-cl-mad-enable -cl-denorms-are-zero -cl-single-precision-constant -cl-no-signed-zeros -cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math -cl-fp32-correctly-rounded-divide-sqrt";
		flags += "-O5";
	}

	// Debug
	bool debug = false; // @ only with Intel
	if (debug) {
		std::string cl_file = "/tmp/" + kernel_name + ".cl";
		flags += " -g -s " + cl_file;
		std::ofstream of(cl_file);
	    of << code;
	    of.close();
	}

	// Compiling to machine code
	err = clBuildProgram(*tsk, 0, NULL, flags.c_str(), NULL, NULL);

	if (err != CL_SUCCESS) {
		cl_build_status status;
		clGetProgramBuildInfo(*tsk, *dev, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);
		size_t log_size;
		clGetProgramBuildInfo(*tsk, *dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = new char [log_size+1];
		clGetProgramBuildInfo(*tsk, *dev, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		std::cout << status << ":" << log_size << std::endl << log << std::endl;
		delete [] log;
	}
	cle::clCheckError(err);

	// Creates 1 kernel per worker, because cl_kernels aren't thread-safe
	for (int j=0; j<Runtime::getConfig().num_ranks; j++) {
		cl_kernel clkrn = clCreateKernel(*tsk, kernel_name.c_str(), &err);
		cle::clCheckError(err);
		tsk.addKernel(clkrn);
	}
}

void Version::copyParams(Version *ver) {
	// Does not copy 'task', makes sure 'dev_type' matches
	assert(deviceType() == ver->deviceType());
	code = ver->code;
	tsk = ver->tsk;
	shared_size = ver->shared_size;
	group_size = ver->group_size;
	num_group = ver->num_group;
	extra_arg = ver->extra_arg;
}

} } // namespace map::detail
