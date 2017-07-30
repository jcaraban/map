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
	ver_to_compile.clear();
}

void Program::addTask(Task *task) {
	task_list.push_back(task);
}

void Program::compose(OwnerClusterList& cluster_list) {
	TimedRegion region(clock,TASKIF);

	// Clusters are transformed into Tasks, in topological order
	for (auto &cluster : cluster_list)
	{
		Task *task = Task::Factory(*this,clock,conf,cluster.get());
		Runtime::getInstance().addTask(task); // Adds task to Runtime
		this->addTask(task); // Adds task to Program
	}
}

void Program::generate() {
	TimedRegion region(clock,CODGEN);

	for (auto task : task_list) // For every task...
	{
		// Generates all the 'versions' first
		task->createVersions();

		// Gets the task 'versions' in a list
		auto ver_list = task->versionList();
		
		for (auto ver : ver_list)  // For every version...
		{
			bool found = ver_cache.find(ver->signature()) != ver_cache.end();
			if (found && conf.compil_cache) // Similar version found in cache
			{
				// Reuses some generated 'code', saves generation / compilation time
				ver->reuseCode(ver_cache[ver->signature()]);
			}
			else // Version not found, has to generate new 'code'
			{
				// Generates the code and creates the cl_program
				ver->generateCode();

				// Adds 'ver' to the list of versions to be compiled
				ver_to_compile.push_back(ver);

				// Adds version to cache, for future 'reusing'
				ver_cache[ver->signature()] = ver;
			}
		}
	}

	print(); // @
}

void Program::compile() {
	TimedRegion region(clock,COMPIL);
	std::vector<std::unique_ptr<std::thread>> vth;

	// Launchs asynchronous threads to compile in parallel
	for (auto ver : ver_to_compile) {
		auto *thr = new std::thread( &Version::compileCode, ver);
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
		std::cout << "  back:" << std::endl;
		for (auto &back : task->backList())
			std::cout << "    " << back << std::endl;
		std::cout << "  forw:" << std::endl;
		for (auto &forw : task->forwList())
			std::cout << "    " << forw << std::endl;
		std::cout << std::endl;
	}
}

} } // namespace map::detail
