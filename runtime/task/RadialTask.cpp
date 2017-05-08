/**
 * @file    RadialTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "RadialTask.hpp"
#include "../Runtime.hpp"
#include "../../util/Direction.hpp"


namespace map { namespace detail {

/*************
   Radial
 *************/

RadialTask::RadialTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{
	for (auto node : group->nodeList()) {
		if (node->pattern().is(RADIAL)) {
			auto *scan = dynamic_cast<RadialScan*>(node);
			assert(scan != nullptr);
			this->scan = scan;
			this->startb = scan->start / blocksize();
			break;
		}
	}
}

void RadialTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();

	// Generates a short list of most promising versions first
	VerkeyList key_list;
	for (int i=0; i<env.deviceSize(); i++) {
		// One version per Radial Direction
		for (int cas=0; cas<N_RADIAL_CASE; cas++) {
			Direction fst, snd;
			radia2dir((RadialCase)cas,fst,snd);
			std::string detail = fst.toString() + snd.toString();

			Verkey key(this);
			key.dev = env.D(i);
			key.group = {std::min(256,blocksize()[0]),1}; // @
			key.detail = detail;
			key_list.push_back(key);
		}
	}

	// Create the versions if they did not exist yet
	for (auto key : key_list) {
		// @
		DeviceType dev_type;
		cl_device_type type = *(cl_device_type*) key.dev.get(CL_DEVICE_TYPE);
		switch (type) {
			case CL_DEVICE_TYPE_CPU:		 dev_type = DEV_CPU; break;
			case CL_DEVICE_TYPE_GPU:		 dev_type = DEV_GPU; break;
			case CL_DEVICE_TYPE_ACCELERATOR: dev_type = DEV_ACC; break;
			default: assert(0);
		}
		//
		const Version *ver = getVersion(dev_type,key.group,key.detail);
		if (ver == nullptr) {
			Version *ver = new Version(key);
			Runtime::getInstance().addVersion(ver); // Adds version to Runtime
			ver_list.push_back(ver);  // Adds version to Task
		}
	}
}

void RadialTask::blocksToLoad(Coord coord, KeyList &in_keys) const {
	Task::blocksToLoad(coord,in_keys);

	// Requires its own output in previous blocks
	for (auto &node : outputList()) {
		if (node->pattern().is(RADIAL))
		{
			auto dif = abs(startb - coord);
			auto unit = sgn(startb - coord);

			auto lambda = [&](Coord nbc) {
				HoldType hold = (sum(abs(startb-nbc)) < sum(dif)) ? HOLD_N : HOLD_0;
				int depend = node->isInput() ? nextInterDepends(node,nbc) : -1; // @
				in_keys.push_back( std::make_tuple(Key(node,nbc),hold,depend) );
			};

			lambda(coord + Coord{unit[0],0}); // Neighbour in first dir
			lambda(coord + Coord{0,unit[1]}); // Neighbour in second dir
			lambda(coord + unit); // Neighbour in first and second dirs
		}
	}
}

void RadialTask::blocksToStore(Coord coord, KeyList &out_keys) const {
	Task::blocksToStore(coord,out_keys);

	// Adds the intra-dependencies
	auto it1 = out_keys.begin();
	for (auto it2=outputList().begin(); it2!=outputList().end(); it1++, it2++)
	{
		if ((*it2)->pattern().is(RADIAL))
		{
			//if (any(coord == 0) || any(coord == numblock()-1)) // @@
			//	continue; // borders have no intra-dependency
			auto dif = abs(startb - coord);
			int dep = all(dif == 0) ? 8 : any(dif == 0) ? 5 : std::abs(dif[0]-dif[1]) ? 3 : 1;
			std::get<2>(*it1) += dep;
		}
	}
}

void RadialTask::initialJobs(std::vector<Job> &job_vec) {
	job_vec.push_back( Job(this,startb) ); // Only start coord
}

void RadialTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);

	// should it check 'for every out_node'? in case of fused Radial?

	auto dif = abs(startb - Coord(done_job.coord));
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			auto nbc = Coord(done_job.coord) + Coord{x,y};
			// Notifies every coord around which is farther to the start than done_job.coord
			if (sum(abs(startb-nbc)) > sum(dif) && all(nbc >= 0) && all(nbc < numblock())) {
				notify(nbc,job_vec);
			}
		}
	}
}

void RadialTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
		notifyAll(job_vec);
	else // Case when prev=D2, self=D2
		notify(done_block.coord,job_vec);
}

int RadialTask::prevInterDepends(Node *node, Coord coord) const {
	return (node->pattern()==INPUT || node->pattern()==FREE) ? 0 : 1;
}

int RadialTask::nextInterDepends(Node *node, Coord coord) const {
	return (node->pattern()==INPUT || node->pattern()==FREE) ? 0 : 1;
}

int RadialTask::prevIntraDepends(Node *node, Coord coord) const {
	if (node->pattern().is(RADIAL))
		return all(coord == startb) ? 0 : any(coord == startb) ? 1 : 3;
	else // non-Radial outputs dont add dependencies
		return 0;
}

int RadialTask::nextIntraDepends(Node *node, Coord coord) const {
	if (node->pattern().is(RADIAL)) {
		if (any(coord == 0) || any(coord == numblock()-1))
			return 0; // borders have no next-intra-dependency
		return all(coord == startb) ? 8 : any(coord == startb) ? 5 : 3;
	} else {
		return 0;
	}
}

void RadialTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const int dim = numdim().toInt();
	auto dif = abs(startb - coord);
	Direction first, second;
	RadialCase rcase;

	// Lambda function
	auto compute_sector = [&](RadialCase rcase)
	{
		std::string detail = radia2str(rcase);
		const Version *ver = getVersion(DEV_ALL,{},detail);

		// CL related vars
		cle::Task tsk = ver->tsk;
		cle::Kernel krn = tsk.K(Tid.rnk());
		cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());
		cl_int err;

		//// Configures kernel

		auto group_size = ver->groupsize();
		auto block_size = blocksize();
		size_t gws[2] = {(size_t)group_size[0],(size_t)group_size[1]};
		size_t lws[2] = {(size_t)group_size[0],(size_t)group_size[1]};
		
		//// Sets kernel arguments

		int arg = 0;
		
		for (auto &b : in_blk) {
			void *dev_mem = (b->entry != nullptr) ? b->entry->dev_mem : nullptr;
			/****/ if (b->holdtype() == HOLD_0) { // If HOLD_0, a null argument is given to the kernel
				clSetKernelArg(*krn, arg++, sizeof(cl_mem), &dev_mem);
				clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.ref());
				clSetKernelArg(*krn, arg++, sizeof(b->fixed), &b->fixed);
			} else if (b->holdtype() == HOLD_1) { // If HOLD_1, a scalar argument is given
				clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.ref());
			} else if (b->holdtype() == HOLD_N) { // In the normal case a valid cl_mem with memory is given
				clSetKernelArg(*krn, arg++, sizeof(cl_mem), &dev_mem);
				clSetKernelArg(*krn, arg++, b->datatype().sizeOf(), &b->value.ref());
				clSetKernelArg(*krn, arg++, sizeof(b->fixed), &b->fixed);
			} else {
				assert(0);
			}
		}
		for (auto b : out_blk)
			clSetKernelArg(*krn, arg++, sizeof(cl_mem), &b->entry->dev_mem);
		for (int i=0; i<dim; i++)
			clSetKernelArg(*krn, arg++, sizeof(int), &block_size[i]);
		for (int i=0; i<dim; i++)
			clSetKernelArg(*krn, arg++, sizeof(int), &coord[i]);
		clSetKernelArg(*krn, arg++, sizeof(int), &group_size[0]);
		for (int i=0; i<dim; i++)
			clSetKernelArg(*krn, arg++, sizeof(int), &scan->start[i]);

		//// Launches kernel
		
		err = clEnqueueNDRangeKernel(*que, *krn, dim, NULL, gws, lws, 0, nullptr, nullptr);
	};

	Runtime::getClock().start(KERNEL);

	if (all(dif == 0)) // Center
	{
		compute_sector(NORTH_EAST);
		compute_sector(EAST_NORTH);
		compute_sector(EAST_SOUTH);
		compute_sector(SOUTH_EAST);
		compute_sector(SOUTH_WEST);
		compute_sector(WEST_SOUTH);
		compute_sector(WEST_NORTH);
		compute_sector(NORTH_WEST);
	}
	else if (sum(dif) == 1)  // Center +- 1 in only 1 dir
	{
		coord2dir(coord[0]-startb[0],coord[1]-startb[1],first,second);

		second = first.rotateLeft();
		rcase = dir2radia(first,second);
		compute_sector(rcase);

		rcase = dir2radia(second,first);
		compute_sector(rcase);

		second = first.rotateRight();
		rcase = dir2radia(first,second);
		compute_sector(rcase);

		rcase = dir2radia(second,first);
		compute_sector(rcase);
	}
	else if (any(dif == 0)) // Compass
	{
		coord2dir(coord[0]-startb[0],coord[1]-startb[1],first,second);

		second = first.rotateLeft();
		rcase = dir2radia(first,second);
		compute_sector(rcase);

		second = first.rotateRight();
		rcase = dir2radia(first,second);
		compute_sector(rcase);
	}
	else if (std::abs(dif[0]-dif[1]) <= 1) // Diagonal (inner and upper too)
	{
		coord2dir(coord[0]-startb[0],coord[1]-startb[1],first,second);

		rcase = dir2radia(first,second);
		compute_sector(rcase);

		rcase = dir2radia(second,first);
		compute_sector(rcase);	
	}
	else // Sector
	{
		coord2dir(coord[0]-startb[0],coord[1]-startb[1],first,second);
		rcase = dir2radia(first,second);
		compute_sector(rcase);

		// @ has to check horizontal cases and rotate the data in shared mem
	}

	cle::Queue que = Runtime::getOclEnv().C(0).Q(Tid.rnk()); // @
	cl_int err = clFinish(*que);
	cle::clCheckError(err);

	Runtime::getClock().stop(KERNEL);
}

} } // namespace map::detail
