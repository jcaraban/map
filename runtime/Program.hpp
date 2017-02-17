/**
 * @file    Program.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_PROGRAM_HPP_
#define MAP_RUNTIME_PROGRAM_HPP_

#include "Config.hpp"
#include "task/Task.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>


namespace map { namespace detail {

class Clock; // Forward declaration

/*
 *
 */
class Program
{
  public:
	Program(Clock &clock, Config &conf);
	~Program() = default;
	Program(const Program&) = delete;
	Program& operator=(const Program&) = delete;

	void clear();

	void compose(OwnerGroupList& group_list);
	void generate();
	void compile();

	void addTask(Task *task);
	const std::vector<Task*>& taskList() const;

	void print();
	
  private:
	Clock &clock; // Aggregate
	Config &conf; // Aggregate

	std::vector<Task*> task_list; //!< List of tasks composing the user program
	std::unordered_map<std::string,Version*> ver_cache; //!< Cache of already generated versions
	VersionList ver_to_comp; //!< List of Versions to be compiled
};

} } // namespace map::detail

#endif
