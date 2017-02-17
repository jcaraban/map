/**
 * @file	OclEnv.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: get() functions are not thread-safe or safe in any way
 */

#include "OclEnv.hpp"

#include <iostream>
#include <limits>
#include <algorithm>
#include <type_traits>
#include <climits>
#include <cstring>
using namespace std;


namespace cle {

/**********************
   OpenCL Environment
 **********************/

OpenclEnvironment::OpenclEnvironment() : m(new mutex) {
	vPlatform.reserve(MAX_PLATFORMS);
	vDevice.reserve(MAX_DEVICES);
	vContext.reserve(MAX_DEVICES);
	vTask.reserve(MAX_DEVICES);
	vKernel.reserve(MAX_DEVICES);
	vQueue.reserve(MAX_DEVICES);
}

OpenclEnvironment::~OpenclEnvironment() {
	clear();
}

void OpenclEnvironment::clear() {
	cl_int err;

	for (auto que : vQueue) {
		err = clReleaseCommandQueue(que.sId);
		clCheckError(err);
	}
	for (auto ker : vKernel) {
		err = clReleaseKernel(ker.sId);
		clCheckError(err);
	}
	for (auto tsk : vTask) {
		err = clReleaseProgram(tsk.sId);
		clCheckError(err);
	}
	for (auto ctx : vContext) {
		err = clReleaseContext(ctx.sId);
		clCheckError(err);
	}

	vQueue.clear();
	vKernel.clear();
	vTask.clear();
	vContext.clear();
	vDevice.clear();
	vPlatform.clear();
}

int OpenclEnvironment::init(const char* str...) {
    va_list args;
    int ret;

    va_start(args, str);
	ret = parse(str, Trace(), args);
	va_end(args);

	return ret;
}

int OpenclEnvironment::addPlatform(cl_platform_id id) {
	lock_guard<mutex> lock(*m); // thread-safe function
	int i;
	
	// Avoids duplications
	for (int j=0; j<(int)vPlatform.size(); j++)
		if (vPlatform[j].sId == id) {
			assert(!"Platform already inserted\n");
			return -1;
		}
	
	i = vPlatform.size();
	vPlatform.resize(i+1);
	vPlatform[i].sId = id;
	return i;
}

int OpenclEnvironment::addDevice(cl_platform_id pid, cl_device_id id) {
	lock_guard<mutex> lock(*m); // thread-safe function
	int i;
	bool found = false;
	
	// Avoids duplications
	for (int j=0; j<(int)vDevice.size(); j++) {
		if (vDevice[j].sId == id) {
			assert(!"Device already inserted\n");
			return -1;
		}
	}

	// Cross references
	for (int j=0; j<(int)vPlatform.size() && !found; j++) {
		if (vPlatform[j].sId == pid) {
			i = vDevice.size();
			vDevice.resize(i+1);
			vPlatform[j].vrDevice.push_back(i);
			vDevice[i].rPlatform = j;
			found = true;
		}
	}
	if (!found) {
		assert(!"No platform found for this device");
		return -1;
	}

	vDevice[i].sId = id;
	return i;
}

int OpenclEnvironment::removeDevice(cl_platform_id pid, cl_device_id id) {
	lock_guard<mutex> lock(*m); // thread-safe function
	bool found = false;
	
	// Un-reference platform
	found = false;
	for (int j=0; j<(int)vPlatform.size() && !found; j++) {
		if (vPlatform[j].sId == pid) {
			for (int k=0; k<(int)vPlatform[j].vrDevice.size(); k++) {
				if (vDevice[vPlatform[j].vrDevice[k]].sId == id) {
					vPlatform[j].vrDevice.erase(vPlatform[j].vrDevice.begin()+k);
					found = true;
				}
			}
		}
	}
	if (!found) {
		assert(!"No platform found");
		return -1;
	}

	// Remove device
	found = false;
	for (int j=0; j<(int)vDevice.size(); j++) {
		if (vDevice[j].sId == id) {
			vDevice.erase(vDevice.begin()+j);
			found = true;
		}
	}
	if (!found) {
		assert(!"No device found");
		return -1;
	}

	return 0;
}

int OpenclEnvironment::addContext(cl_platform_id pid, cl_device_id* did, cl_uint ndev, cl_context id) {
	lock_guard<mutex> lock(*m); // thread-safe function
	int i, p=0;
	bool found = false;
	
	// Avoids duplications
	for (int j=0; j<(int)vContext.size(); j++) {
		if (vContext[j].sId == id) {
			assert(!"Context already inserted\n");
			return -1;
		}
	}
	
	// Cross references
	for (int j=0; j<(int)vPlatform.size() && !found; j++) {
		if (vPlatform[j].sId == pid) {
			i = vContext.size();
			vContext.resize(i+1);
			vPlatform[j].vrContext.push_back(i);
			vContext[i].rPlatform = j;
			found = true;
			p = j;
		}
	}
	if (!found) {
		assert(!"No platform found for this context");
		return -1;
	}
	
	for (int k=0; k<ndev; k++) {
		found = false;
		for (int j=0; j<(int)vPlatform[p].vrDevice.size() && !found; j++) {
			if (platform(p).device(j).id() == did[k]) {
				vDevice[ vPlatform[p].vrDevice[j] ].vrContext.push_back(i);
				vContext[i].vrDevice.push_back(j);
				found = true;
			}
		}
		if (!found)
			break;
	}
	if (!found) {
		assert(!"No devices found for this context");
		vContext.resize(i); // Deletes the allocated object
		return -1;
	}
	
	vContext[i].sId = id;
	return i;
}

int OpenclEnvironment::addTask(cl_context cid, cl_program id) {
	lock_guard<mutex> lock(*m); // thread-safe function
	int i;
	bool found = false;
	
	// Avoids duplications
	for (i=0; i<(int)vTask.size(); i++)
		if (vTask[i].sId == id) {
			assert(!"Task already inserted\n");
			return -1;
		}
			
	// Cross references
	for (int j=0; j<(int)vContext.size() && !found; j++)
		if (vContext[j].sId == cid) {
			i = vTask.size();
			vTask.resize(i+1);
			vContext[j].vrTask.push_back(i);
			vTask[i].rContext = j;
			found = true;
		}
	if (!found) {
		assert(!"No context found for this task");
		return -1;
	}
	
	vTask[i].sId = id;
	return i;
}

int OpenclEnvironment::addKernel(cl_program pid, cl_kernel id) {
	lock_guard<mutex> lock(*m); // thread-safe function
	int i;
	bool found = false;
	
	// Avoids duplications
	for (i=0; i<(int)vKernel.size(); i++)
		if (vKernel[i].sId == id) {
			assert(!"Kernel already inserted\n");
			return -1;
		}
			
	// Cross references
	for (int j=0; j<(int)vTask.size() && !found; j++)
		if (vTask[j].sId == pid) {
			i = vKernel.size();
			vKernel.resize(i+1);			
			vTask[j].vrKernel.push_back(i);
			vKernel[i].rTask = j;
			found = true;
		}
	if (!found) {
		assert(!"No task found for this kernel");
		return -1;
	}
	
	vKernel[i].sId = id;
	return i;
}

int OpenclEnvironment::addQueue(cl_context cid, cl_device_id did, cl_command_queue id) {
	lock_guard<mutex> lock(*m); // thread-safe function
	int i, c, d;
	bool found = false;
	
	// Avoids duplications
	for (int j=0; j<(int)vQueue.size(); j++) {
		if (vQueue[j].sId == id) {
			assert(!"Queue already inserted\n");
			return -1;
		}
	}
	
	// Cross references
	for (int j=0; j<(int)vContext.size() && !found; j++) {
		if (vContext[j].sId == cid) {
			found = true;
			c = j;
		}
	}
	if (!found) {
		assert(!"No context found for this queue");
		return -1;
	}
	
	found = false;
	for (int j=0; j<(int)vContext[c].vrDevice.size(); j++) {
		d = vContext[c].vrDevice[j];
		if (vDevice[d].sId == did) {
			found = true;
		}
	}
	if (!found) {
		assert(!"No device found for this queue");
		return -1;
	}

	i = vQueue.size();
	vQueue.resize(i+1);
	vQueue[i].rContext = c;
	vQueue[i].rDevice = d;
	vContext[c].vrQueue.push_back(i);
	vDevice[d].vrQueue.push_back(i);
	
	vQueue[i].sId = id;
	return i;
}

Platform OpenclEnvironment::platform(int i) {
	assert(i < platformSize());
	return Platform(this,i);
}

int OpenclEnvironment::platformSize() {
	return vPlatform.size();
}

Device OpenclEnvironment::device(int i) {
	assert(i < deviceSize());
	return Device(this,i);
}

int OpenclEnvironment::deviceSize() {
	return vDevice.size();
}

Context OpenclEnvironment::context(int i) {
	assert(i < contextSize());
	return Context(this,i);
}

int OpenclEnvironment::contextSize() {
	return vContext.size();
}

Task OpenclEnvironment::task(int i) {
	assert(i < taskSize());
	return Task(this,i);
}

int OpenclEnvironment::taskSize() {
	return vTask.size();
}

Kernel OpenclEnvironment::kernel(int i) {
	assert(i < kernelSize());
	return Kernel(this,i);
}

int OpenclEnvironment::kernelSize() {
	return vKernel.size();
}

Queue OpenclEnvironment::queue(int i) {
	assert(i < queueSize());
	return Queue(this,i);
}

int OpenclEnvironment::queueSize() {
	return vQueue.size();
}

Platform OpenclEnvironment::P(int i) { return platform(i); }
int OpenclEnvironment::nP() { return platformSize(); }
Device OpenclEnvironment::D(int i) { return device(i); }
int OpenclEnvironment::nD() { return deviceSize(); }
Context OpenclEnvironment::C(int i) { return context(i); }
int OpenclEnvironment::nC() { return contextSize(); }
Task OpenclEnvironment::T(int i) { return task(i); }
int OpenclEnvironment::nT() { return taskSize(); }
Kernel OpenclEnvironment::K(int i) { return kernel(i); }
int OpenclEnvironment::nK() { return kernelSize(); }
Queue OpenclEnvironment::Q(int i) { return queue(i); }
int OpenclEnvironment::nQ() { return queueSize(); }

/************
   Platform
 ************/

Platform::Platform(OpenclEnvironment *father, int plt) :
	father(father),
	ref(plt)
{ }

Platform::Platform() :
	father(nullptr),
	ref(-1)
{ }

int Platform::init(const char* str...) {
    va_list args;
    int ret;

    va_start(args, str);
	ret = parse(str, Trace(), args);
	va_end(args);

	return ret;
}

int Platform::addDevice(cl_device_id dev_id) {
	int ret = father->addDevice(id(), dev_id);
	return (ret < 0) ? ret : deviceSize() - 1;
}

int Platform::removeDevice(cl_device_id dev_id) {
	int ret = father->removeDevice(id(), dev_id);
	return (ret < 0) ? ret : deviceSize() - 1;
}

int Platform::addContext(cl_device_id* dev_id, cl_uint ndev, cl_context ctx_id) {
	int ret = father->addContext(id(), dev_id, ndev, ctx_id);
	return (ret < 0) ? ret : contextSize() - 1;
}

const OpenclEnvironment& Platform::environment() const {
	return *father;
}

OpenclEnvironment& Platform::environment() {
	return *father;
}

Device Platform::device(int i) {
	assert(i < deviceSize());
	return father->device(father->vPlatform[ref].vrDevice[i]);
}

int Platform::deviceSize() {
	return father->vPlatform[ref].vrDevice.size();
}

Context Platform::context(int i) {
	assert(i < contextSize());
	return father->context(father->vPlatform[ref].vrContext[i]);
}

int Platform::contextSize() {
	return father->vPlatform[ref].vrContext.size();
}

const cl_platform_id& Platform::id() const {
	return father->vPlatform[ref].sId;
}

const cl_platform_id& Platform::operator*() const {
	return id();
}

const void* Platform::get(cl_platform_info info) const {
	static char buf[BUF_SIZE];
	cl_int error = clGetPlatformInfo(id(), info, BUF_SIZE, buf, NULL);
	clCheckError(error);
	return buf;
}

Device Platform::D(int i) { return device(i); }
int Platform::nD() { return deviceSize(); }
Context Platform::C(int i) { return context(i); }
int Platform::nC() { return contextSize(); }

/**********
   Device
 **********/

Device::Device(OpenclEnvironment *father, int dev) :
	father(father),
	ref(dev)
{ }

Device::Device() :
	father(nullptr),
	ref(-1)
{ }

int Device::init(const char* str...) {
    va_list args;
    int ret;

    va_start(args, str);
	ret = parse(str, Trace(), args);
	va_end(args);

	return ret;
}

int Device::addContext(cl_context ctx_id) {
	cl_device_id dev[] = {id()};
	int ret = father->addContext(platform().id(), dev, 1, ctx_id);
	return (ret < 0) ? ret : contextSize() - 1;
}

int Device::addQueue(cl_context ctx_id, cl_command_queue que_id) {
	int ret = father->addQueue(ctx_id, id(), que_id);
	return (ret < 0) ? ret : queueSize() - 1;
}

const OpenclEnvironment& Device::environment() const {
	return *father;
}

OpenclEnvironment& Device::environment() {
	return *father;
}

Platform Device::platform() {
	return father->platform(father->vDevice[ref].rPlatform);
}

Context Device::context(int i) {
	assert(i < contextSize());
	return father->context(father->vDevice[ref].vrContext[i]);
}

int Device::contextSize() {
	return father->vDevice[ref].vrContext.size();
}

Queue Device::queue(int i) {
	assert(i < queueSize());
	return father->queue(father->vDevice[ref].vrQueue[i]);
}

int Device::queueSize() {
	return father->vDevice[ref].vrQueue.size();
}

const cl_device_id& Device::id() const {
	return father->vDevice[ref].sId;
}

const cl_device_id& Device::operator*() const {
	return id();
}

const void* Device::get(cl_device_info info) const {
	static char buf[BUF_SIZE];
	cl_int error = clGetDeviceInfo(id(), info, BUF_SIZE, buf, NULL);
	clCheckError(error);
	return buf;
}

Platform Device::P() { return platform(); }
Context Device::C(int i) { return context(i); }
int Device::nC() { return contextSize(); }
Queue Device::Q(int i) { return queue(i); }
int Device::nQ() { return queueSize(); }

/***********
   Context
 ***********/

Context::Context(OpenclEnvironment *father, int ctx) :
	father(father),
	ref(ctx)
{ }

Context::Context() :
	father(nullptr),
	ref(-1)
{ }

int Context::init(const char* str...) {
    va_list args;
    int ret;

    va_start(args, str);
	ret = parse(str, Trace(), args);
	va_end(args);

	return ret;
}

int Context::addTask(cl_program tsk_id) {
	int ret = father->addTask(id(), tsk_id);
	return (ret < 0) ? ret : taskSize() - 1;
}

int Context::addQueue(cl_device_id dev_id, cl_command_queue que_id) {
	int ret = father->addQueue(id(), dev_id, que_id);
	return (ret < 0) ? ret : queueSize() - 1;
}

const OpenclEnvironment& Context::environment() const {
	return *father;
}

OpenclEnvironment& Context::environment() {
	return *father;
}

Platform Context::platform() {
	return father->platform(father->vContext[ref].rPlatform);
}

Device Context::device(int i) {
	assert(i < deviceSize());
	return father->device(father->vContext[ref].vrDevice[i]);
}

int Context::deviceSize() {
	return father->vContext[ref].vrDevice.size();
}

Task Context::task(int i) {
	assert(i < taskSize());
	return father->task(father->vContext[ref].vrTask[i]);
}

int Context::taskSize() {
	return father->vContext[ref].vrTask.size();
}

Queue Context::queue(int i) {
	assert(i < queueSize());
	return father->queue(father->vContext[ref].vrQueue[i]);
}

int Context::queueSize() {
	return father->vContext[ref].vrQueue.size();
}

const cl_context& Context::id() const {
	return father->vContext[ref].sId;
}

const cl_context& Context::operator*() const {
	return id();
}

Platform Context::P() { return platform(); }
Device Context::D(int i) { return device(i); }
int Context::nD() { return deviceSize(); }
Task Context::T(int i) { return task(i); }
int Context::nT() { return taskSize(); }
Queue Context::Q(int i) { return queue(i); }
int Context::nQ() { return queueSize(); }

/***********
   Task
 ***********/

Task::Task(OpenclEnvironment *father, int tsk) :
	father(father),
	ref(tsk)
{ }

Task::Task() :
	father(nullptr),
	ref(-1)
{ }

int Task::addKernel(cl_kernel krn_id) {
	int ret = father->addKernel(id(), krn_id);
	return (ret < 0) ? ret : kernelSize() - 1;
}

const OpenclEnvironment& Task::environment() const {
	return *father;
}

OpenclEnvironment& Task::environment() {
	return *father;
}

Context Task::context() {
	return father->context(father->vTask[ref].rContext);
}

Kernel Task::kernel(int i) {
	assert(i < kernelSize());
	return father->kernel(father->vTask[ref].vrKernel[i]);
}

int Task::kernelSize() {
	return father->vTask[ref].vrKernel.size();
}

const cl_program& Task::id() const {
	return father->vTask[ref].sId;
}

const cl_program& Task::operator*() const {
	return id();
}

Context Task::C() { return context(); }
Kernel Task::K(int i) { return kernel(i); }
int Task::nK() { return kernelSize(); }

/**********
   Kernel
 **********/

Kernel::Kernel(OpenclEnvironment *father, int krn) :
	father(father),
	ref(krn)
{ }

Kernel::Kernel() :
	father(nullptr),
	ref(-1)
{ }

const OpenclEnvironment& Kernel::environment() const {
	return *father;
}

OpenclEnvironment& Kernel::environment() {
	return *father;
}

Task Kernel::task() {
	return father->task(father->vKernel[ref].rTask);
}

const cl_kernel& Kernel::id() const {
	return father->vKernel[ref].sId;
}

const cl_kernel& Kernel::operator*() const {
	return id();
}

Task Kernel::T() { return task(); }

/*********
   Queue
 *********/

Queue::Queue(OpenclEnvironment *father, int que) :
	father(father),
	ref(que)
{ }

Queue::Queue() :
	father(nullptr),
	ref(-1)
{ }

const OpenclEnvironment& Queue::environment() const {
	return *father;
}

OpenclEnvironment& Queue::environment() {
	return *father;
}

Device Queue::device() {
	return father->device(father->vQueue[ref].rDevice);
}

Context Queue::context() {
	return father->context(father->vQueue[ref].rContext);
}

const cl_command_queue& Queue::id() const {
	return father->vQueue[ref].sId;
}

const cl_command_queue& Queue::operator*() const {
	return id();
}

Device Queue::D() { return device(); }
Context Queue::C() { return context(); }

/***********
   Parsing
 ***********/

int OpenclEnvironment::parse(const char* str, Trace trace, va_list args) {
	char *head, *token, *end, *ptr, *cond;
	const char *tail;
	int desired_items, idx, aux, ret = 0;
	size_t size_head, size_cond;

	// Fills array with all available platforms
	const int max_items = MAX_PLATFORMS;
	cl_platform_id array[max_items];
	cl_uint n_items;
	cl_int error;

	error = clGetPlatformIDs(MAX_PLATFORMS, array, &n_items);
	clCheckError(error);

	// Splits query into head and tail
	tail = strpbrk(str, ",;");
	if (tail != NULL)
		tail++;
	size_head = (tail != NULL) ? tail-str : strlen(str)+1;
	head = new char [size_head];
	strncpy(head, str, size_head);
	head[size_head-1] = '\0';

	//  Parses first token
	token = strtok(head, " ");
	assert(token != NULL);
	assert(strncmp(token,"P=",2) == 0);
	if (token[2] == '#') {
		assert(strlen(token) == 3);
		desired_items = max_items;
	} else if (token[2] == '%') {
		assert(strlen(token) == 4);
		if (token[3] == 'd' || token[1] == 'i') {
			desired_items = va_arg(args, int);
			assert(desired_items > 0);
		} else if (token[3] == 'u') {
			desired_items = va_arg(args, uint);
		} else {
			assert(!"Type not accepted");
		}
	} else if (token[2] >= '0' && token[2] <= '9') {
		desired_items = strtol(token+2, &end, 10);
		assert(desired_items > 0 && desired_items <= MAX_PLATFORMS);
		assert(strlen(token) == (size_t)(end-token));
	} else {
		assert(!"Expression not accepted");
	}

	// Sets the pointer where conditions start if there is any, set it NULL otherwise
	ptr = token + strlen(token) + 1;
	if (ptr >= head+size_head) {
		cond = NULL;
	} else {
		size_cond = strlen(ptr) + 1;
		cond = new char [size_cond];
	}

	// Filters platforms by given conditions
	for (int i=0; i<n_items; i++) {
		bool accepted = true;

		va_list args_copy;
		va_copy(args_copy, args);

		if (cond != NULL) {
			strncpy(cond, ptr, size_cond);
			cond[size_cond-1] = '\0';
			accepted = evaluateConditions(array[i], cond, args_copy);
		}

		// If accepted, recursively parses with tail and reduces number of plats
		if (accepted && desired_items > 0) {
	    	idx = this->addPlatform(array[i]);
	    	trace.platform = idx;

		    if (tail != NULL) {
	    		aux = this->platform(idx).parse(tail, trace, args_copy);
	    		if (aux < ret)
	    			ret = aux;

		    }
			desired_items--;
		}

		va_end(args_copy);
	}

	delete [] cond;
    delete [] head;

	return ret;
}

bool OpenclEnvironment::evaluateConditions(cl_platform_id id, char* cond, va_list args) {
	char buf[BUF_SIZE];
	char *token, *left, *right;
	bool equals = true;
	cl_int error;

	// Right now it only does a logic 'and' with all conditions. It should be extended...

	// Iterates the tokens, returning false if the platform does not match the conditions
	token = strtok(cond, " ");
    while (equals && token != NULL) {
    	left = token;
    	assert(left[0] == 'P' && left[1] == '_');
    	right = strpbrk(token, "=");
    	assert(right != NULL);
    	*right = '\0';
    	right++;

		error = clGetPlatformInfo(id, str_to_platinfo(left), BUF_SIZE, buf, NULL); // <- Null is ret_size, could become an issue?
		clCheckError(error);
		buf[BUF_SIZE-1] = '\0';

		if (right[0] == '%') {
			assert(strlen(right) == 2);
			if (right[1] == 's') {
				equals = (equals && strstr(buf, va_arg(args,char*)) != NULL);
			} else if (right[1] == 'd' || right[1] == 'i') {
				equals = (equals && *(reinterpret_cast<int*>(buf)) == va_arg(args,int));
			} else if (right[1] == 'u') {
				equals = (equals && *(reinterpret_cast<uint*>(buf)) == va_arg(args,uint));
			} else {
				assert(!"Type not accepted");
			}
		} else {
			equals = (equals && strstr(buf,right) != NULL);
		}

		token = strtok(NULL, " ");
    }
	
	return equals;
}

int Platform::parse(const char* str, Trace trace, va_list args) {
	char *head, *token, *end, *ptr, *cond;
	const char *tail;
	int desired_items, idx, aux, ret = 0;
	size_t size_head, size_cond;

	// Fills array with all available platforms
	const int max_items = MAX_DEVICES;
	cl_device_id array[max_items];
	cl_uint n_items;
	cl_int error;

	error = clGetDeviceIDs(this->id(), CL_DEVICE_TYPE_ALL, MAX_DEVICES, array, &n_items);
	clCheckError(error);

	// Splits query into head and tail
	tail = strpbrk(str, ",;");
	if (tail != NULL)
		tail++;
	size_head = (tail != NULL) ? tail-str : strlen(str)+1;
	head = new char [size_head];
	strncpy(head, str, size_head);
	head[size_head-1] = '\0';

	//  Parses first token
	token = strtok(head, " ");
	assert(token != NULL);
	assert(strncmp(token,"D=",2) == 0);
	if (token[2] == '#') {
		assert(strlen(token) == 3);
		desired_items = max_items;
	} else if (token[2] == '%') {
		assert(strlen(token) == 4);
		if (token[3] == 'd' || token[1] == 'i') {
			desired_items = va_arg(args, int);
			assert(desired_items > 0);
		} else if (token[3] == 'u') {
			desired_items = va_arg(args, uint);
		} else {
			assert(!"Type not accepted");
		}
	} else if (token[2] >= '0' && token[2] <= '9') {
		desired_items = strtol(token+2, &end, 10);
		assert(desired_items > 0);// && desired_items!=LONG_MAX && desired_items!=LONG_MIN);
		assert(strlen(token) == end-token);
	} else {
		assert(!"Expression not accepted");
	}

	// Sets the pointer where conditions start if there is any, set it NULL otherwise
	ptr = token + strlen(token) + 1;
	if (ptr >= head+size_head) {
		cond = NULL;
	} else {
		size_cond = strlen(ptr) + 1;
		cond = new char [size_cond];
	}

	// Filters platforms by given conditions
	for (int i=0; i<n_items; i++) {
		bool accepted = true;

		va_list args_copy;
		va_copy(args_copy, args);

		if (cond != NULL) {
			strncpy(cond, ptr, size_cond);
			cond[size_cond-1] = '\0';
			accepted = evaluateConditions(array[i], cond, args_copy);
		}

		// If accepted, recursively parses with tail and reduces number of desired devs
		if (accepted && desired_items > 0) {
	    	idx = father->addDevice(this->id(), array[i]);
	    	trace.device = idx;

		    if (tail != NULL) {
	    		//aux = father->device(idx).parse(tail, trace, args_copy);
	    		aux = father->device(idx).parse(tail, trace, args_copy);
	    		if (aux < ret)
	    			ret = aux;

		    }
			desired_items--;
		}

		va_end(args_copy);
	}

	delete [] cond;
    delete [] head;

	return ret;
}

bool Platform::evaluateConditions(cl_device_id id, char* cond, va_list args) {
	char buf[BUF_SIZE];
	char *token, *left, *right;
	bool equals = true;
	cl_int error;

	// Right not it only does a logic 'and' with all conditions. It should be extended to have 'or', but using a real parser (spirit)

	// Iterates the tokens, returning false if the platform does not match the conditions
	token = strtok(cond, " ");
    while (equals && token != NULL) {
    	left = token;
    	assert(left[0] == 'D' && left[1] == '_');
    	right = strpbrk(token, "=");
    	assert(right != NULL);
    	*right = '\0';
    	right++;

		error = clGetDeviceInfo(id, str_to_devinfo(left), BUF_SIZE, buf, NULL); // <- Null is ret_size, issue?
		clCheckError(error);
		buf[BUF_SIZE-1] = '\0';

		if (right[0] == '%') {
			assert(strlen(right) == 2);
			if (right[1] == 's') {
				equals = (equals && strstr(buf, va_arg(args,char*)) != NULL);
			} else if (right[1] == 'd' || right[1] == 'i') {
				equals = (equals && *(reinterpret_cast<int*>(buf)) == va_arg(args,int));
			} else if (right[1] == 'u') {
				equals = (equals && *(reinterpret_cast<uint*>(buf)) == va_arg(args,uint));
			} else {
				assert(!"Type not accepted");
			}
		} else {
			equals = (equals && strstr(buf,right) != NULL);
		}

		token = strtok(NULL, " ");
    }
	
	return equals;
}

int Device::parse(const char* str, Trace trace, va_list args) {
	char *head, *token;
	const char *tail;
	int size, idx, aux, ret = 0;

	// CL variables
	cl_device_id dev_group[1];
	cl_context_properties cp[3] = {CL_CONTEXT_PLATFORM, 0, 0};
	cl_context context;
	cl_int error;
	cp[1] = (cl_context_properties) this->platform().id();

	// Splits query into head and tail
	tail = strpbrk(str, ",;");
	if (tail != NULL)
		tail++;
	size = (tail != NULL) ? tail-str : strlen(str)+1;
	head = new char [size];
	memcpy(head, str, size*sizeof(char));
	head[size-1] = '\0';

	//  Parses first token
	token = strtok(head, " ");
	assert(token != NULL);
	assert(token[0] == 'C' && token[1] == '=');
	token += 2;
	if (strcmp(token,"1xP") == 0)
	{
		assert(!"Not implemented yet");
	}
	else if (strcmp(token,"1xD") == 0)
	{
		// Adds one context to the device
		dev_group[0] = this->id();
		context = clCreateContext(cp, 1, dev_group, NULL, NULL, &error);
		clCheckError(error);

		idx = father->addContext(father->platform(trace.platform).id(), dev_group, 1, context);
		trace.context = idx;
	}
	else {
		assert(!"No valid case");
	}
    
	// Recursively parses with tail
    if (tail != NULL) {
		aux = father->context(idx).parse(tail, trace, args);
		if (aux < ret)
			ret = aux;
    }

	delete [] head;

	return ret;
}

int Context::parse(const char* str, Trace trace, va_list args) {
	char *head, *token;
	const char *tail;
	int size, idx, aux, ret = 0;

	// CL variables
	cl_program program;
	cl_int error;

	// Splits query into head and tail
	tail = strpbrk(str, ",;");
	if (tail != NULL)
		tail++;
	size = (tail != NULL) ? tail-str : strlen(str)+1;
	head = new char [size];
	memcpy(head, str, size*sizeof(char));
	head[size-1] = '\0';

	//  Parses first token
	token = strtok(head, " ");
	assert(token != NULL);
	assert(token[0] == 'T' && token[1] == '=');
	if (token[2] == '%' && token[3] == 's')
	{
		char *src = va_arg(args,char*);
		program = clCreateProgramWithSource(this->id(), 1, (const char **) &src, NULL, &error);
		clCheckError(error);
		error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
		clCheckError(error);

		idx = father->addTask(this->id(), program);
		trace.task = idx;

		// Extracts function name (necessary for clCreateKernel)
		std::string copy(src);
		token = strtok((char*)copy.data(), " \n");
		token = strtok(NULL, " \n");
		token = strtok(NULL, "(");
		trace.task_name = std::string(token);
	}
	else {
		assert(!"No valid case");
	}
    
	// Recursively parses with tail
    if (tail != NULL) {
		aux = father->task(idx).parse(tail, trace, args);
		if (aux < ret)
			ret = aux;
    }

	delete [] head;

	return ret;
}

int Task::parse(const char* str, Trace trace, va_list args) {
	char *head, *token;
	const char *tail;
	int size, idx, aux, ret = 0;

	// CL variables
	cl_kernel kernel;
	cl_int error;

	// Splits query into head and tail
	tail = strpbrk(str, ",;");
	if (tail != NULL)
		tail++;
	size = (tail != NULL) ? tail-str : strlen(str)+1;
	head = new char [size];
	memcpy(head, str, size*sizeof(char));
	head[size-1] = '\0';

	//  Parses first token
	token = strtok(head, " ");
	assert(token != NULL);
	assert(token[0] == 'K' && token[1] == '=');
	token += 2;
	if (strcmp(token, "1") == 0)
	{
		kernel = clCreateKernel(this->id(), trace.task_name.c_str(), &error);
		std::cout << trace.task_name.c_str() << std::endl;
		clCheckError(error);

		idx = father->addKernel(this->id(), kernel);
		trace.kernel = idx;
	}
	else {
		assert(!"No valid case");
	}

	// Recursively parses with tail
    if (tail != NULL) {
		aux = father->kernel(idx).parse(tail, trace, args);
		if (aux < ret)
			ret = aux;
    }

	delete [] head;

	return ret;
}

int Kernel::parse(const char* str, Trace trace, va_list args) {
	char *head, *token;
	const char *tail;
	int size, idx, aux, ret = 0;

	// CL variables
	cl_command_queue queue;
	cl_int error;

	// Splits query into head and tail
	tail = strpbrk(str, ",;");
	if (tail != NULL)
		tail++;
	size = (tail != NULL) ? tail-str : strlen(str)+1;
	head = new char [size];
	memcpy(head, str, size*sizeof(char));
	head[size-1] = '\0';

	//  Parses first token
	token = strtok(head, " ");
	assert(token != NULL);
	assert(token[0] == 'Q' && token[1] == '=');
	token += 2;
	if (strcmp(token, "1") == 0)
	{
		queue = clCreateCommandQueue(father->context(trace.context).id(), father->device(trace.device).id(), 0, &error);
		clCheckError(error);

		idx = father->addQueue(father->context(trace.context).id(), father->device(trace.device).id(), queue);
		trace.queue = idx;
	}
	else {
		assert(!"No valid case");
	}

	delete [] head;

	return ret;
}

/*********
   Utils
 *********/

void checkError(cl_int error, const char* file, const char* func, int line) {
	if (error != 0) {
		cerr << error_to_str(error) << " in " << file << ":" << func << ":" << line << endl;
		//cerr << "Press ENTER to continue or Ctrl+C to exit...";
		//cin.ignore(numeric_limits<streamsize>::max(), '\n');
		assert(0);
	}
}

cl_platform_info str_to_platinfo(const char* str) {
	if      (strcmp(str, "P_PROFILE"   )==0) return CL_PLATFORM_PROFILE;
	else if (strcmp(str, "P_VERSION"   )==0) return CL_PLATFORM_VERSION;
	else if (strcmp(str, "P_NAME"      )==0) return CL_PLATFORM_NAME;
	else if (strcmp(str, "P_VENDOR"    )==0) return CL_PLATFORM_VENDOR;
	else if (strcmp(str, "P_EXTENSIONS")==0) return CL_PLATFORM_EXTENSIONS;
	else assert(false);
}

cl_device_info str_to_devinfo(const char* str) {
	if      (strcmp(str, "D_ADDRESS_BITS")==0) return CL_DEVICE_ADDRESS_BITS;
	else if (strcmp(str, "D_AVAILABLE")==0) return CL_DEVICE_AVAILABLE;
	else if (strcmp(str, "D_COMPILER_AVAILABLE")==0) return CL_DEVICE_COMPILER_AVAILABLE;
	else if (strcmp(str, "D_ENDIAN_LITTLE")==0) return CL_DEVICE_ENDIAN_LITTLE;
	else if (strcmp(str, "D_ERROR_CORRECTION_SUPPORT")==0) return CL_DEVICE_ERROR_CORRECTION_SUPPORT;
	else if (strcmp(str, "D_EXECUTION_CAPABILITIES")==0) return CL_DEVICE_EXECUTION_CAPABILITIES;
	else if (strcmp(str, "D_EXTENSIONS")==0) return CL_DEVICE_EXTENSIONS;
	else if (strcmp(str, "D_GLOBAL_MEM_CACHE_SIZE")==0) return CL_DEVICE_GLOBAL_MEM_CACHE_SIZE;
	else if (strcmp(str, "D_GLOBAL_MEM_CACHE_TYPE")==0) return CL_DEVICE_GLOBAL_MEM_CACHE_TYPE;
	else if (strcmp(str, "D_GLOBAL_MEM_CACHELINE_SIZE")==0) return CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE;
	else if (strcmp(str, "D_GLOBAL_MEM_SIZE")==0) return CL_DEVICE_GLOBAL_MEM_SIZE;
	else if (strcmp(str, "D_IMAGE_SUPPORT")==0) return CL_DEVICE_IMAGE_SUPPORT;
	else if (strcmp(str, "D_IMAGE2D_MAX_HEIGHT")==0) return CL_DEVICE_IMAGE2D_MAX_HEIGHT;
	else if (strcmp(str, "D_IMAGE2D_MAX_WIDTH")==0) return CL_DEVICE_IMAGE2D_MAX_WIDTH;
	else if (strcmp(str, "D_IMAGE3D_MAX_DEPTH")==0) return CL_DEVICE_IMAGE3D_MAX_DEPTH;
	else if (strcmp(str, "D_IMAGE3D_MAX_HEIGHT")==0) return CL_DEVICE_IMAGE3D_MAX_HEIGHT;
	else if (strcmp(str, "D_IMAGE3D_MAX_WIDTH")==0) return CL_DEVICE_IMAGE3D_MAX_WIDTH;
	else if (strcmp(str, "D_LOCAL_MEM_SIZE")==0) return CL_DEVICE_LOCAL_MEM_SIZE;
	else if (strcmp(str, "D_LOCAL_MEM_TYPE")==0) return CL_DEVICE_LOCAL_MEM_TYPE;
	else if (strcmp(str, "D_MAX_CLOCK_FREQUENCY")==0) return CL_DEVICE_MAX_CLOCK_FREQUENCY;
	else if (strcmp(str, "D_MAX_COMPUTE_UNITS")==0) return CL_DEVICE_MAX_COMPUTE_UNITS;
	else if (strcmp(str, "D_MAX_CONSTANT_ARGS")==0) return CL_DEVICE_MAX_CONSTANT_ARGS;
	else if (strcmp(str, "D_MAX_CONSTANT_BUFFER_SIZE")==0) return CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE;
	else if (strcmp(str, "D_MAX_MEM_ALLOC_SIZE")==0) return CL_DEVICE_MAX_MEM_ALLOC_SIZE;
	else if (strcmp(str, "D_MAX_PARAMETER_SIZE")==0) return CL_DEVICE_MAX_PARAMETER_SIZE;
	else if (strcmp(str, "D_MAX_READ_IMAGE_ARGS")==0) return CL_DEVICE_MAX_READ_IMAGE_ARGS;
	else if (strcmp(str, "D_MAX_SAMPLERS")==0) return CL_DEVICE_MAX_SAMPLERS;
	else if (strcmp(str, "D_MAX_WORK_GROUP_SIZE")==0) return CL_DEVICE_MAX_WORK_GROUP_SIZE;
	else if (strcmp(str, "D_MAX_WORK_ITEM_DIMENSIONS")==0) return CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS;
	else if (strcmp(str, "D_MAX_WORK_ITEM_SIZES")==0) return CL_DEVICE_MAX_WORK_ITEM_SIZES;
	else if (strcmp(str, "D_MAX_WRITE_IMAGE_ARGS")==0) return CL_DEVICE_MAX_WRITE_IMAGE_ARGS;
	else if (strcmp(str, "D_MEM_BASE_ADDR_ALIGN")==0) return CL_DEVICE_MEM_BASE_ADDR_ALIGN;
	else if (strcmp(str, "D_MIN_DATA_TYPE_ALIGN_SIZE")==0) return CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE;
	else if (strcmp(str, "D_NAME")==0) return CL_DEVICE_NAME;
	else if (strcmp(str, "D_PLATFORM")==0) return CL_DEVICE_PLATFORM;
	else if (strcmp(str, "D_PREFERRED_VECTOR_WIDTH_CHAR")==0) return CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR;
	else if (strcmp(str, "D_PREFERRED_VECTOR_WIDTH_DOUBLE")==0) return CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE;
	else if (strcmp(str, "D_PREFERRED_VECTOR_WIDTH_FLOAT")==0) return CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT;
	else if (strcmp(str, "D_PREFERRED_VECTOR_WIDTH_INT")==0) return CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT;
	else if (strcmp(str, "D_PREFERRED_VECTOR_WIDTH_LONG")==0) return CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG;
	else if (strcmp(str, "D_PREFERRED_VECTOR_WIDTH_SHORT")==0) return CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT;
	else if (strcmp(str, "D_PROFILE")==0) return CL_DEVICE_PROFILE;
	else if (strcmp(str, "D_PROFILING_TIMER_RESOLUTION")==0) return CL_DEVICE_PROFILING_TIMER_RESOLUTION;
	else if (strcmp(str, "D_QUEUE_PROPERTIES")==0) return CL_DEVICE_QUEUE_PROPERTIES;
	else if (strcmp(str, "D_SINGLE_FP_CONFIG")==0) return CL_DEVICE_SINGLE_FP_CONFIG;
	else if (strcmp(str, "D_TYPE")==0) return CL_DEVICE_TYPE;
	else if (strcmp(str, "D_VENDOR_ID")==0) return CL_DEVICE_VENDOR_ID;
	else if (strcmp(str, "D_VENDOR")==0) return CL_DEVICE_VENDOR;
	else if (strcmp(str, "D_VERSION")==0) return CL_DEVICE_VERSION;
	else assert(false);
}

const char * error_to_str(cl_int err){
	switch (err) {
	case 0  : return "CL_SUCCESS";
	case -1 : return "CL_DEVICE_NOT_FOUND";
	case -2 : return "CL_DEVICE_NOT_AVAILABLE";
	case -3 : return "CL_COMPILER_NOT_AVAILABLE";
	case -4 : return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
	case -5 : return "CL_OUT_OF_RESOURCES";
	case -6 : return "CL_OUT_OF_HOST_MEMORY";
	case -7 : return "CL_PROFILING_INFO_NOT_AVAILABLE";
	case -8 : return "CL_MEM_COPY_OVERLAP";
	case -9 : return "CL_IMAGE_FORMAT_MISMATCH";
	case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
	case -11: return "CL_BUILD_PROGRAM_FAILURE";
	case -12: return "CL_MAP_FAILURE";
	case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
	case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
	case -15: return "CL_COMPILE_PROGRAM_FAILURE";
	case -16: return "CL_LINKER_NOT_AVAILABLE";
	case -17: return "CL_LINK_PROGRAM_FAILURE";
	case -18: return "CL_DEVICE_PARTITION_FAILED";
	case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

	case -30: return "CL_INVALID_VALUE";
	case -31: return "CL_INVALID_DEVICE_TYPE";
	case -32: return "CL_INVALID_PLATFORM";
	case -33: return "CL_INVALID_DEVICE";
	case -34: return "CL_INVALID_CONTEXT";
	case -35: return "CL_INVALID_QUEUE_PROPERTIES";
	case -36: return "CL_INVALID_COMMAND_QUEUE";
	case -37: return "CL_INVALID_HOST_PTR";
	case -38: return "CL_INVALID_MEM_OBJECT";
	case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
	case -40: return "CL_INVALID_IMAGE_SIZE";
	case -41: return "CL_INVALID_SAMPLER";
	case -42: return "CL_INVALID_BINARY";
	case -43: return "CL_INVALID_BUILD_OPTIONS";
	case -44: return "CL_INVALID_PROGRAM";
	case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
	case -46: return "CL_INVALID_KERNEL_NAME";
	case -47: return "CL_INVALID_KERNEL_DEFINITION";
	case -48: return "CL_INVALID_KERNEL";
	case -49: return "CL_INVALID_ARG_INDEX";
	case -50: return "CL_INVALID_ARG_VALUE";
	case -51: return "CL_INVALID_ARG_SIZE";
	case -52: return "CL_INVALID_KERNEL_ARGS";
	case -53: return "CL_INVALID_WORK_DIMENSION";
	case -54: return "CL_INVALID_WORK_GROUP_SIZE";
	case -55: return "CL_INVALID_WORK_ITEM_SIZE";
	case -56: return "CL_INVALID_GLOBAL_OFFSET";
	case -57: return "CL_INVALID_EVENT_WAIT_LIST";
	case -58: return "CL_INVALID_EVENT";
	case -59: return "CL_INVALID_OPERATION";
	case -60: return "CL_INVALID_GL_OBJECT";
	case -61: return "CL_INVALID_BUFFER_SIZE";
	case -62: return "CL_INVALID_MIP_LEVEL";
	case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
	case -64: return "CL_INVALID_PROPERTY";
	case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
	case -66: return "CL_INVALID_COMPILER_OPTIONS";
	case -67: return "CL_INVALID_LINKER_OPTIONS";
	case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

	case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
	case -1001: return "no_valid_ICDs_found";
	case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
	case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
	case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
	case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";

	default: return "Unknown OpenCL error";
	}
}

} // namespace cle