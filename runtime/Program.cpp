/**
 * @file    Program.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: clBuildProgram is sequential, using threads does not help
 *
 * TODO: perhaps change std::thread to std::async and std::future
 */

#include "Program.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "skeleton/Skeleton.hpp"
#include "Runtime.hpp"
#include <memory>
#include <thread>


namespace map { namespace detail {

/***********
   Program
 ***********/

Program::Program(Clock &clock, Config &conf)
	: clock(clock)
	, conf(conf)
{ }

void Program::clear() {
	task_list.clear();
	// Note: ver_cache is not cleared
	ver_to_comp.clear();
}

void Program::addTask(Task *task) {
	task_list.push_back(task);
}

void Program::compose(OwnerGroupList& group_list) {
	TimedRegion region(clock,TASKIF);

	// Groups are transformed into Tasks, in topological order
	for (auto &group : group_list)
	{
		Task *task = Task::Factory(group.get()); // Creates a Task out of the group
		Runtime::getInstance().addTask(task); // Adds task to Runtime
		this->addTask(task); // Adds task to Program
	}
}

void Program::generate() {
	TimedRegion region(clock,CODGEN);

	for (auto task : task_list) // For every task...
	{
		if (task->numdim() == D0)
			continue; // Skips D0 tasks, those dont require kernels

		VersionList ver_list = task->versionList(); // Gets task's versions
		
		for (auto ver : ver_list)  // For every version...
		{
			auto it = ver_cache.find(ver->signature());
			if (it != ver_cache.end() && conf.compil_cache) // Similar version found in cache
			{
				ver->copyParams(it->second);
			}
			else // Version code not found, has to generate it
			{
				// Ask Skel::Factory() for the appropiate skeleton
				auto skel = std::unique_ptr<Skeleton>( Skeleton::Factory(ver) );

				// Generates the code and configures the version
				skel->generate();

				// Creates the cl_task (aka cl_program)
				ver->createProgram();

				// Adds version to cache
				ver_cache[ver->signature()] = ver;

				// Adds 'ver' to the list of versions to be compiled
				ver_to_comp.push_back(ver);
			}
		}
	}

	print();
}

void Program::compile() {
	TimedRegion region(clock,COMPIL);
	std::vector<std::unique_ptr<std::thread>> vth;

	// Launchs asynchronous threads to compile in parallel
	for (auto ver : ver_to_comp) {
		auto *thr = new std::thread( &Version::compileProgram, ver);
		vth.push_back( std::unique_ptr<std::thread>(thr) );
	}

	// Waits for threads to finish
	for (auto &thread : vth)
		thread->join();
}

const std::vector<Task*>& Program::taskList() const {
	return task_list;
}

void Program::print() {
	// prints tasks
	std::cout << "--- Tasks --- " << task_list.size() << std::endl;
	for (auto &task : task_list) {
		std::cout << task->pattern() << "  " << task->id() << std::endl;
		std::cout << "  NumDim: " << task->numdim().toString() << ", DataSize: " << task->datasize()
			<< ", BlockSize: " << task->blocksize() << ", NumBlock: " << task->numblock()
			<< std::endl;
		std::cout << "InList: " << task->inputList().size() << ", OutList: " << task->outputList().size() << std::endl;
		std::cout << "  prev:" << std::endl;
		for (auto &prev : task->prevList())
			std::cout << "    " << prev << std::endl;
		std::cout << "  next:" << std::endl;
		for (auto &next : task->nextList())
			std::cout << "    " << next << std::endl;
		std::cout << std::endl;
	}
}

} } // namespace map::detail
