/**
 * @file    Runtime.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Using singleton pattern to wrap the set of unique vars used by the runtime
 *
 * TODO? add OutputList to enable eval(all_nodes) ?
 * TODO? changing the big structures to pointers allows avoiding their #include
 */

#ifndef MAP_RUNTIME_HPP_
#define MAP_RUNTIME_HPP_

#include "Program.hpp"
#include "Cache.hpp"
#include "Scheduler.hpp"
#include "Worker.hpp"
#include "Clock.hpp"
#include "Config.hpp"
#include "visitor/SimplifierOnline.hpp"
#include "../cle/cle.hpp"

#include <string>
#include <list>
#include <set>
#include <memory>
#include <thread>


namespace map { namespace detail {

struct Node; //!< Forward declaration
struct Group; //!< Forward declaration
struct Task; //!< Forward declaration
struct Version; //!< Forward declaration

typedef std::vector<std::unique_ptr<Node>> OwnerNodeList;
typedef std::vector<std::unique_ptr<Group>> OwnerGroupList;
typedef std::vector<std::unique_ptr<Task>> OwnerTaskList;
typedef std::vector<std::unique_ptr<Version>> OwnerVersionList;
typedef std::vector<Node*> NodeList;
typedef std::set<Node*> NodeSet;

enum LoopMode { NORMAL_MODE, LOOP_START, LOOP_BODY, LOOP_AGAIN };

struct LoopStruct {
	NodeList prev; //!< Previous existing nodes used in the loop
	NodeList cond; //!< Nodes expressing the halt condition
	NodeList body; //!< Nodes composing the main loop body
	NodeList again; //!< Again the body, to find feedbacks
	NodeList feed_in; //!< Nodes that feedback into body (input side)
	NodeList feed_out; //!< (output side), repeteadly swap with (feed_in)
	Node *loop;
	NodeList head; //!< Head nodes created by Loop
	NodeList tail; //!< Tail nodes created by Loop
};

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
	void work(); // stars a series of threads to work
	void reportEval(); // prints execution time whitin 'eval'
	void reportOver(); // prints overall execution times

	void workflow(NodeList list); // Executes the list of nodes

  public:
	static Runtime& getInstance();
	static Config& getConfig();
	static Clock& getClock();
	static cle::OclEnv& getOclEnv();

	void setupDevices(std::string plat_name, DeviceType dev, std::string dev_name);

	Node* loopDigestion(bool start, bool body, bool again, bool end);
	void loopClear();
	void loopCondition(Node *node);
	void loopAddNode(Node *node);
	Node* loopAssemble();
	void loopAgainTail(Node *loop, Node ***agains, Node ***tails, int *num);

	Node* addNode(Node *node);
	Task* addTask(Task *task);
	Group* addGroup(Group *group);
	Version* addVersion(Version *ver);

	void removeNode(Node *node); // @
	void updateNode(Node *node); // @
	void unlinkIsolated(const OwnerNodeList &node_list, bool drop=false);

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

	OwnerNodeList node_list; //!< Full list of nodes added to the runtime during the script execution (EDAG)
	OwnerGroupList group_list; //!< 1 fused list is valid for 1 evaluation (GDAG)
	OwnerTaskList task_list; //!< 1 task list is valid for 1 evaluation (TDAG)
	OwnerVersionList ver_list; //!< List of code versions required by the tasks

	SimplifierOnline simplifier; //!<

	LoopMode loop_mode; //!< Indicates if we are inside a (possibly nested) loop
	int loop_level; //!< Loop nesting level, tracks the recursion, from 0..16
	std::vector<LoopStruct> loop_struct; //!< Stores the ongoing (nested?) loop
};

} } // namespace map::detail

#endif
