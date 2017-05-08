/**
 * @file    Version.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Version.hpp"
#include "task/Task.hpp"
#include "skeleton/Skeleton.hpp"
#include "Runtime.hpp"
#include <fstream>


namespace map { namespace detail {

Verkey::Verkey(Task *task) 
	: task(task)
{ }

Verkey::Verkey(Version *ver)
	: task(ver->task)
	, dev(ver->dev)
	, group(ver->group_size)
	, detail(ver->detail)
{ }

bool Verkey::operator==(const Verkey &k) const {
	return task==k.task && dev==k.dev && all(group==k.group) && detail.compare(k.detail)==0;
}

std::size_t Verkey::Hash::operator()(const Verkey &k) const {
	std::size_t h = std::hash<const Task*>()(k.task);
	h ^= k.dev.hash();
	h ^= coord_hash()(k.group);
	h ^= std::hash<std::string>()(k.detail);
	return h;
}

Version::Version(Verkey key)
	: task(key.task)
	, dev(key.dev)
	, group_size(key.group)
	, detail(key.detail)
{
	// Filling 'dev_type'
	dev_type = cledev2devtype(dev);
	// Filling 'signature'
	ver_sign = task->group()->signature() + detail + std::to_string(deviceType());
	// Filling rest...
	shared_size = -1;
	num_group = (task->blocksize() - 1) / groupsize() + 1;

	assert(all(task->blocksize() % groupsize() == 0)); // 'groupsize' must divide 'blocksize' exactly!
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

const GroupSize& Version::groupsize() const {
	return group_size;
}

const NumGroup& Version::numgroup() const {
	return num_group;
}

void Version::generateCode() {
	// Asks Skel::Factory() for the appropiate skeleton
	auto skel = std::unique_ptr<Skeleton>( Skeleton::Factory(this) );

	// Generates the code and configures the version
	code = skel->generate();
	assert(not code.empty());

	// Parameters... // @

	// Creates the cl_task (i.e. cl_program)
	createProgram();
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

void Version::compileCode() {
	std::string flags;
	cl_int err;

	size_t hash = std::hash<std::string>()( signature() );
	std::string kernel_name = "krn" + std::to_string(hash);

	// Includes
	#ifdef RAND123 // needs the RAND123 dir because OpenCL compiles at runtime,
		#define xstr(s) str(s)
		#define str(s) #s
		flags += "-I " xstr(RAND123) " ";
		#undef str
		#undef xstr
	#endif

	// Optimizations
	bool opt = false; // only with AMD
	if (opt) {
		flags += " -cl-mad-enable -cl-denorms-are-zero -cl-single-precision-constant -cl-no-signed-zeros ";
		flags += " -cl-unsafe-math-optimizations -cl-finite-math-only -cl-fast-relaxed-math ";
		flags += " -cl-fp32-correctly-rounded-divide-sqrt ";
		flags += " -O5 ";
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

void Version::reuseCode(Version *ver) {
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
