/**
 * @file    Runtime.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Using singleton pattern to wrap the set of unique vars used by the runtime
 *
 * TODO? create "Context" class and move all active variables there
 * TODO? add OutputList to enable eval(all_nodes) ?
 * TODO? changing the big structures to pointers allows avoiding their #include
 */

#ifndef MAP_RUNTIME_HPP_
#define MAP_RUNTIME_HPP_

#include "frontend/LoopAssembler.hpp"
#include "backend/Program.hpp"
#include "runtime/Cache.hpp"
#include "runtime/Scheduler.hpp"
#include "runtime/Worker.hpp"
#include "visitor/Simplifier.hpp"
#include "cle/cle.hpp"
#include "Clock.hpp"
#include "Config.hpp"

#include <string>
#include <list>
#include <set>
#include <memory>
#include <thread>


namespace map { namespace detail {

struct Node; //!< Forward declaration
struct Cluster; //!< Forward declaration
struct Task; //!< Forward declaration
struct Version; //!< Forward declaration

typedef std::vector<std::unique_ptr<Node>> OwnerNodeList;
typedef std::vector<std::unique_ptr<Cluster>> OwnerClusterList;
typedef std::vector<std::unique_ptr<Task>> OwnerTaskList;
typedef std::vector<std::unique_ptr<Version>> OwnerVersionList;
typedef std::vector<Node*> NodeList;

/*
 * Runtime class
 * Used as a wrapper for the global variables 'node_list', 'conf', etc...
 * Using Singleton pattern
 */
class Runtime
{
	Runtime();                                  // Private constructor
	~Runtime();                                 // Private destructor
	Runtime(const Runtime &P) = delete;         // Not Implemented
	void operator=(const Runtime &P) = delete;  // Not Implemented

	void clear(); // runtime structures are cleared
	void reportEval(); // prints execution time whitin 'eval'
	void reportOver(); // prints overall execution times

	void workflow(NodeList list); // Executes the list of nodes
	void work(); // stars a series of threads to work

  public:
	static Runtime& getInstance();
	static cle::OclEnv& getOclEnv();
	static Config& getConfig();
	static Clock& getClock();
	static Cache& getCache();
	static Program& getProgram();
	static LoopAssembler& getLoopAssembler();

	void setupDevices(std::string plat_name, DeviceType dev, std::string dev_name);
	Node* loopAssemble();
	
	Node* addNode(Node *node);
	Cluster* addCluster(Cluster *cluster);
	Task* addTask(Task *task);
	Version* addVersion(Version *ver);

	void print_nodes(const OwnerNodeList &list); // @
	void print_nodes(const NodeList &list); // @
	void long_print_nodes(const OwnerNodeList &list); // @
	void long_print_nodes(const NodeList &list); // @

	void evaluate(NodeList list); // Evaluate a list of {0,1,N} nodes 

  private:
	cle::OclEnv clenv; //!< OpenCL environment
	Config conf; //!< Framework configuration
	Clock clock; //!< Timers & counters
	Program program; //!< 1 program is valid for 1 evaluation
	Cache cache; //!< Memory cache, allocates and releases memory (chunks 1xScript, subBuffers 1xeval)
	Scheduler scheduler; //!< Job scheduler
	std::vector<Worker> workers; //!< Vector of workers
	std::vector<std::unique_ptr<std::thread>> threads; //!< Vector of threads
	
	int id_count; // Counter that give unique SSA ids to the nodes

	OwnerNodeList node_list; //!< Full list of nodes added to the runtime during the script execution
	OwnerClusterList cluster_list; //!< 1 fused list is valid for 1 evaluation
	OwnerTaskList task_list; //!< 1 task list is valid for 1 evaluation
	OwnerVersionList ver_list; //!< List of code versions required by the tasks

	Simplifier simplifier; //!<
	LoopAssembler assembler;
};

} } // namespace map::detail

#endif
