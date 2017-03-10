/**
 * @file	Runtime.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: is a nested simplifier necessary for the loop?
 * TODO: in loopAssemble, is_included can be avoided by tagging the loop nodes upon insertion
 */

#include "Runtime.hpp"
#include "dag/util.hpp"
#include "dag/Loop.hpp"
#include "visitor/Lister.hpp"
#include "visitor/Sorter.hpp"
#include "visitor/Partitioner.hpp"
#include "visitor/Fusioner.hpp"
#include "visitor/Exporter.hpp"
#include "visitor/Cloner.hpp"


namespace map { namespace detail {

/***************
   Definitions
****************/

thread_local ThreadId Tid;  //!< Per thread local storage for the ID = {node,device,rank}

/************
   Runtime
*************/

Runtime& Runtime::getInstance() {
	static Runtime instance; // Guaranteed to be destroyed. Instantiated on first use.
    return instance;
}

Config& Runtime::getConfig() {
	return getInstance().conf;
}

Clock& Runtime::getClock() {
	return getInstance().clock;
}

cle::OclEnv& Runtime::getOclEnv() {
	return getInstance().clenv;
}

Runtime::Runtime()
	: clenv()
	, conf()
	, clock(conf)
	, program(clock,conf)
	, cache(program,clock,conf)
	, scheduler(program,clock,conf)
	, workers()
	, threads()
	, node_list()
	, group_list()
	, task_list()
	, simplifier(node_list)
	, loop_mode(NORMAL_MODE)
	, loop_level(-1)
{
	// Overall process timing
	clock.start(OVERALL);

	// Workers construction
	for (int i=0; i<conf.max_num_workers; i++) {
		workers.emplace_back(cache,scheduler,clock,conf);
	}

	// Initialize loop supporting structures
	loop_struct.resize(conf.nested_loop_limit);

	//// something else could be initialized here ////
}

Runtime::~Runtime() {
	// something could be deleted here

	// Nodes cannot be deleted until unlinked
	unlinkIsolated(node_list);

	cache.freeChunks();
	clock.stop(OVERALL);

	reportOver();
}

void Runtime::clear() {
	node_list.clear();
	group_list.clear();
	task_list.clear();
	program.clear();
	cache.clear();
	scheduler.clear();
	Node::id_count = 0;
}

void Runtime::setupDevices(std::string plat_name, DeviceType dev, std::string dev_name) {
	// Free memory cache. Does nothing if chunks were not allocated yet
	cache.freeChunks();

	// Times related to preparing OpenCL to use the parallel devices
	clock.start(DEVICES);

	// Free old queues, programs, kernels, contexts, etc
	clenv.clear();

	// Only 1 device accepted
	clenv.init("P=# P_NAME=%s, D=1 D_TYPE=%d D_NAME=%s", plat_name.data(), dev, dev_name.data()); //, C=1xD

	// Apply device fission if: 'Intel' and 'CPU' and 'conf.interpreted'
	if (plat_name.compare("Intel")==0 && dev==DEV_CPU && conf.interpreted)
	{
		assert(conf.num_ranks == 1);
		cl_device_partition_property dprops[5] = // Reduce CPU to 1 device with 1 thread
			{CL_DEVICE_PARTITION_BY_COUNTS, 1, CL_DEVICE_PARTITION_BY_COUNTS_LIST_END, 0};
		cl_device_id subdev_id[1];
		cl_int err;
		err = clCreateSubDevices(*clenv.D(0), dprops, 1, subdev_id, NULL);
		cle::clCheckError(err);
		// Remove original CPU device, add new sub-device
		clenv.P(0).removeDevice(*clenv.D(0));
		clenv.P(0).addDevice(subdev_id[0]);
	}

	// Adding 1 context
	cl_device_id dev_group[1] = {*clenv.D(0)};
	cl_context_properties cp[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)*clenv.P(0), 0};
	cl_int err;
	cl_context ctx = clCreateContext(cp, 1, dev_group, NULL, NULL, &err);
	cle::clCheckError(err);
	clenv.D(0).addContext(ctx);

	// Adding queues to the contexts
	for (int i=0; i<clenv.nC(); i++) {
		cle::Context ctx = clenv.C(i);
		for (int j=0; j<ctx.nD(); j++) {
			cle::Device dev = ctx.D(j);
			for (int k=0; k<conf.num_ranks; k++) {
				cl_int err;
				cl_command_queue que = clCreateCommandQueue(*ctx, *dev, 0, &err);
				cle::clCheckError(err);
				ctx.addQueue(*dev, que);
			}
		}
	}

	clock.stop(DEVICES);

	// Initializing memory cache. Allocates chunks of memory now
	cache.allocChunks(clenv.C(0));
}

Node* Runtime::loopDigestion(bool start, bool body, bool again, bool end) {
	Node *node = nullptr;
	LoopStruct &loop = loop_struct[loop_level];

	if (start) // Starts the process of digesting a loop
	{
		assert(loop_mode == NORMAL_MODE); // @ not covering nested case
		loop_level++;
		loopClear();
		loop_mode = LOOP_START;
	}
	else if (body) // The condition has been digested, next is the body
	{
		assert(loop_mode == LOOP_START);
		assert(not loop.cond.empty());
		loop_mode = LOOP_BODY;
	}
	else if (again) // Digested the body again to find the feedback links
	{
		assert(loop_mode == LOOP_BODY);
		assert(not loop.body.empty());
		loop_mode = LOOP_AGAIN;
	}
	else if (end) // Ends the digestion and assembles the final Loop node
	{
		assert(loop_mode == LOOP_AGAIN);
		loop_level--;
		loop_mode = NORMAL_MODE;
	}
	else {
		assert(0);
	}
	return node;
}

void Runtime::loopClear() {
	loop_struct[loop_level].prev.clear();
	loop_struct[loop_level].cond.clear();
	loop_struct[loop_level].body.clear();
	loop_struct[loop_level].again.clear();
	loop_struct[loop_level].feed_in.clear();
	loop_struct[loop_level].feed_out.clear();
	loop_struct[loop_level].head.clear();
	loop_struct[loop_level].tail.clear();
}

void Runtime::loopAddNode(Node *node) {
	LoopStruct &loop = loop_struct[loop_level];
	// Stores nodes first, assembles them later
	if (loop_mode == LOOP_START)
	{
		loop.cond.push_back(node);
	}
	else if (loop_mode == LOOP_BODY)
	{
		loop.body.push_back(node);
	}
	else if (loop_mode == LOOP_AGAIN)
	{
		loop.again.push_back(node);
	}
	else {
		assert(0);
	}
}

void Runtime::loopCondition(Node *node) {
	loop_struct[loop_level].cond.push_back(node);
}

Node* Runtime::loopAssemble() {
	assert(loop_mode == LOOP_AGAIN);
	LoopStruct &loop = loop_struct[loop_level];

	// Note: the order of the nodes inside the lists is important,
	//       do not apply optimizations (e.g. set) that break that order !!

	assert(loop.cond.size() == 2);
	Node *cond_node = loop.cond[0]; // 'cond' node that activated the Python while loop
	loop.prev.push_back(loop.cond[0]); // 'cond' is the first 'prev' and thus first 'feed_in'
	loop.feed_out.push_back(loop.cond[1]); // 'again-cond' is the first 'feed_out'

	// 'prev' of 'loop' are all those 'prev' to 'body', but outside 'body'
	for (auto node : loop.body)
		for (auto prev : node->prevList())
			if (!is_included(prev,loop.body) && !is_included(prev,loop.prev))
				loop.prev.push_back(prev);

	// The constant 'prev' are the 'prev' of 'again' outside 'body'+'again'
	NodeSet const_set; // No need for order here, so std::set is ok
	for (auto node : loop.again)
		for (auto prev : node->prevList())
			if (!is_included(prev,loop.body) && !is_included(prev,loop.again))
				const_set.insert(prev);
	NodeList const_list = NodeList(const_set.begin(),const_set.end());

	// All 'body' nodes that only depend on the 'cons_list' are loop invariant nodes
	int i = 0;
	while (i < loop.body.size()) {
		Node *node = loop.body[i++];
		bool invar = true;
		for (auto prev : node->prevList())
			if (invar && !is_included(prev,const_list))
				invar = false;
		// Detected a loop invariant, moves it out of 'body'
		if (invar) {
			remove_value(node,loop.body);
			loop.prev.push_back(node);
			const_list.push_back(node);
			i--;
		}
		// @@ test this code better
	}

	// 'feed_in' nodes are those 'prevs' not found on the constant list
	loop.feed_in = left_join(loop.prev,const_list);

	// 'feed_out' nodes are those reused between iterations
	for (auto node : loop.again)
		for (auto prev : node->prevList())
			if (is_included(prev,loop.body) && !is_included(prev,loop.feed_out))
				loop.feed_out.push_back(prev);

	// Note: 'feed_in' and 'feed_out' must maintain same size and --> order <--
	assert(loop.feed_in.size() == loop.feed_out.size());

	// Unlinks 'again' nodes, now that we have figured out the feedbacks
	for (auto it=loop.again.rbegin(); it!=loop.again.rend(); it++) {
		Node *node = *it;
		// Nobody should link here
		assert(node->nextList().empty());
		// Inform prev nodes
		for (auto &prev : node->prevList())
			prev->removeNext(node);
		node->prev_list.clear();
		// The 'again' nodes are not deleted just yet, Python still points to them.
		// With loopAgainTail() Python updates the loop variables to 'tail' nodes
		// which lets the garbage collector delete the 'again' nodes, sometime later
		Node::id_count--; // @ I'd be fired for this
	}

	// 'loop' node creation, insertion, simplification
	Node *node = Loop::Factory(loop.prev,cond_node,loop.body,loop.feed_in,loop.feed_out);
	node_list.push_back( std::unique_ptr<Node>(node) );

	// TODO: how to simplify a loop?
	Node *orig = node; // = simplifier.simplify(node);

	// Ask 'loop' for its newly created 'head' and 'tail' nodes
	loop.loop = orig;
	Loop *loop_node = dynamic_cast<Loop*>(orig);

	// Inserts the 'cond' / 'head' / 'feed' in/out / 'tail' nodes into the node_list
	node_list.push_back( std::unique_ptr<Node>(loop_node->cond_node) );
	for (auto node : loop_node->head_list)
		node_list.push_back( std::unique_ptr<Node>(node) );
	for (auto node : loop_node->feed_in_list)
		node_list.push_back( std::unique_ptr<Node>(node) );
	for (auto node : loop_node->feed_out_list)
		node_list.push_back( std::unique_ptr<Node>(node) );
	for (auto node : loop_node->tail_list)
		node_list.push_back( std::unique_ptr<Node>(node) );

	// 'head' nodes are not used at the moment, maybe in the future?
	loop.head.reserve(loop_node->headList().size());
	for (auto head : loop_node->headList())
		loop.head.push_back(head);
	// 'tail' nodes are sent back to Python to update its variables
	loop.tail.reserve(loop_node->tailList().size());
	for (auto tail : loop_node->tailList())
		loop.tail.push_back(tail);

	return orig;
}

void Runtime::loopAgainTail(Node *node, Node ***agains, Node ***tails, int *num) {
	LoopStruct &loop = loop_struct[loop_level];
	assert(loop.loop == node);

	*agains = loop.again.data();
	*tails = loop.tail.data();
	*num = loop.again.size();
	assert(*num <= loop.tail.size());
}

void Runtime::work() {
	TimedRegion region(clock,EXEC);

	threads.clear();

	// Threads spawn, each with a worker
	int i = 0;
	for (int n=0; n<conf.num_machines; n++) {
		for (int d=0; d<conf.num_devices; d++) {
			for (int r=0; r<conf.num_ranks; r++) {
				auto thr = new std::thread(&Worker::work, &workers[i], ThreadId(n,d,r));
				threads.push_back( std::unique_ptr<std::thread>(thr) );
				i++;
			}
		}
	}

	// Workers gathering
	for (auto &thr : threads)
		thr->join();
}

Node* Runtime::addNode(Node *node) {
	// TimedRegion region(clock,ADDNODE);
	Node *orig;
	if (loop_mode == NORMAL_MODE)  // Not inside a loop
	{
		node_list.push_back( std::unique_ptr<Node>(node) );
		orig = simplifier.simplify(node);
	}
	else // Inside a (possibly nested) loop
	{
		node_list.push_back( std::unique_ptr<Node>(node) );
		orig = simplifier.simplify(node);
		// Dont add node to loop if:
		//   - it was repeated (i.e. orig != node)
		//   - the node is FREE (i.e. read, const)
		if (orig == node && orig->pattern()!=FREE)
			loopAddNode(orig);
	}
	return orig;
}

Group* Runtime::addGroup(Group *group) {
	group_list.push_back( std::unique_ptr<Group>(group) );
	return group;
}

Task* Runtime::addTask(Task *task) {
	task_list.push_back( std::unique_ptr<Task>(task) );
	return task;
}

Version* Runtime::addVersion(Version *ver) {
	ver_list.push_back( std::unique_ptr<Version>(ver) );
	return ver;
}

void Runtime::removeNode(Node *node) {
	simplifier.drop(node);
}

void Runtime::updateNode(Node *node) {
	Node *orig = simplifier.simplify(node);
	assert(orig == node);
}

void Runtime::unlinkIsolated(const OwnerNodeList &node_list, bool drop) {
	// In reverse order, new isolated appear as we unlink nodes
	for (auto it=node_list.rbegin(); it!=node_list.rend(); it++) {
		Node *node = it->get();
		// Skip referred nodes
		if (node->ref > 0)
			continue;
		// Drop from Simplifier
		if (drop)
			simplifier.drop(node);
		// Inform prev nodes
		for (auto &prev : node->prevList())
			prev->removeNext(node);
		node->prev_list.clear();
	}
}

void Runtime::evaluate(NodeList list_to_eval) {
	assert(clenv.contextSize() == 1);

	// Prepares the clock for another round
	clock.prepare();
	clock.start(EVAL);

	// Unlinks all unaccessible (i.e. isolated) nodes & removes them from simplifier 
	unlinkIsolated(node_list,true);

	// Cleaning of old unaccessible nodes
	auto pred = [](std::unique_ptr<Node> &node){ return node->ref==0; };
	node_list.erase(std::remove_if(node_list.begin(),node_list.end(),pred),node_list.end());

	NodeList full_list;
	if (list_to_eval.size() == 0) // eval all nodes
	{
		full_list.reserve(node_list.size());
		for (auto &node : node_list)
			full_list.push_back(node.get());
	}
	else if (list_to_eval.size() == 1) // eval one node
	{
		full_list = Lister().list(list_to_eval.front());
	}
	else if (list_to_eval.size() >= 2) // eval few nodes
	{
		full_list = Lister().list(list_to_eval);
	}

	// Sorts the list by 'dependencies' 1st, and 'id' 2nd
	auto sort_list = Sorter().sort(full_list);

// @ Prints nodes 4rd
std::cout << "----" << std::endl;
for (auto node : sort_list)
	std::cout << node->id << "\t" << node->getName() << "\t " << node->ref << std::endl;
std::cout << "----" << std::endl;

	// ... continue ... make clone keep the ids

	// Clones the list of sorted nodes into new list of new nodes
	OwnerNodeList priv_list; //!< Owned by this particular evaluation
	auto cloner = Cloner(priv_list);
	auto graph = cloner.clone(sort_list);
	auto map_new_old = cloner.new_hash;

// @ Prints nodes 5rd
std::cout << "----" << std::endl;
for (auto &node : priv_list)
	std::cout << node->id << "\t" << node->getName() << "\t " << node->ref << std::endl;
std::cout << "----" << std::endl;

	/**/
	workflow(graph);
	/**/

	// Transfers scalar values to original nodes
	for (auto node : graph)
		if (node->numdim() == D0 && node->pattern().isNot(FREE))
			map_new_old.find(node)->second->value = node->value;

	// Unlinks the private nodes before they are deleted
	unlinkIsolated(priv_list);

	clock.stop(EVAL);
	reportEval();
}

void Runtime::workflow(NodeList list) {
	// Variables reused for every pipeline pass
	group_list.clear();
	task_list.clear();
	program.clear();
	scheduler.clear();

	// Task fusion: fusing nodes into groups
	Fusioner fusioner(group_list);
	fusioner.fuse(list);

	// Export DAG
	Exporter exporter(fusioner.group_list_of);
	exporter.exportDag(list);
	
	// Program tasks composition
	program.compose(group_list);

	// Parallel code generation
	program.generate();

	// Code compilation
	program.compile();

	// Allocation of cache entries
	cache.allocEntries();
	
	// Adding initial jobs
	scheduler.addInitialJobs();

	// Make workers work
	this->work();

	// Release of cache entries
	cache.freeEntries();
}

void Runtime::reportEval() {
	// Synchronizes all times up till the system level
	clock.syncAll({ID_ALL,ID_ALL,ID_ALL});
	const int W = conf.num_workers;
	const double V = clock.get(EVAL) / 100;
	const double E = clock.get(EXEC) / 100;

	std::cerr.setf(std::ios::fixed);
	std::cerr.precision(2);

	std::cerr << "Eval:    " << clock.get(EVAL) << "s" << std::endl;
	//std::cerr << " Simplif: " << clock.get("Simplif")/V << "%" << std::endl;
	std::cerr << " Fusion:  " << clock.get(FUSION)/V << "%" << std::endl;
	std::cerr << " Taskif:  " << clock.get(TASKIF)/V << "%" << std::endl;
	std::cerr << " CodGen:  " << clock.get(CODGEN)/V << "%" << std::endl;
	std::cerr << " Compil:  " << clock.get(COMPIL)/V << "%" << std::endl;
	std::cerr << " Add Job: " << clock.get(ADD_JOB)/V << "%" << std::endl;
	std::cerr << " AllocE:  " << clock.get(ALLOC_E)/V << "%" << std::endl;
	std::cerr << " Exec:    " << clock.get(EXEC)/V << "%" << std::endl;
	std::cerr << " FreeE:   " << clock.get(FREE_E)/V << "%" << std::endl;

	std::cerr << "  get job: " << clock.get(GET_JOB)/W/E << "%" << std::endl;
	std::cerr << "  load:    " << clock.get(LOAD)/W/E << "%" << std::endl;
	std::cerr << "  compute: " << clock.get(COMPUTE)/W/E << "%" << std::endl;
	std::cerr << "  store:   " << clock.get(STORE)/W/E << "%" << std::endl;
	std::cerr << "  notify:  " << clock.get(NOTIFY)/W/E << "%" << std::endl;
	
	std::cerr << "    read:  " << clock.get(READ)/W/E << "%" << std::endl;
	std::cerr << "    send:  " << clock.get(SEND)/W/E << "%" << std::endl;
	std::cerr << "    kernl: " << clock.get(KERNEL)/W/E << "%" << std::endl;
	std::cerr << "    recv:  " << clock.get(RECV)/W/E << "%" << std::endl;
	std::cerr << "    write: " << clock.get(WRITE)/W/E << "%" << std::endl;

	const size_t L = clock.get(LOADED) + clock.get(NOT_LOADED);
	const size_t S = clock.get(STORED) + clock.get(NOT_STORED);
	const size_t C = clock.get(COMPUTED) + clock.get(NOT_COMPUTED);
	std::cerr << "  loaded: " << clock.get(LOADED) << " (" << clock.get(NOT_LOADED) << ") " << clock.get(LOADED)/(double)L*100 << "%" << std::endl;
	std::cerr << "  stored: " << clock.get(STORED) << " (" << clock.get(NOT_STORED) << ") " << clock.get(STORED)/(double)S*100 << "%" << std::endl;
	std::cerr << "  computed: " << clock.get(COMPUTED) << " (" << clock.get(NOT_COMPUTED) << ") " << clock.get(COMPUTED)/(double)C*100 << "%" << std::endl;
	std::cerr << "  discarded: " << clock.get(DISCARDED) << " evicted: " << clock.get(EVICTED) << std::endl;

	std::cerr << (char*)clenv.D(0).get(CL_DEVICE_NAME) << std::endl;
}

void Runtime::reportOver() {
	// Synchronizes all times up till the system level
	clock.syncAll({ID_ALL,ID_ALL,ID_ALL});
	const double O = clock.get(OVERALL) / 100;

	std::cerr.setf(std::ios::fixed);
	std::cerr.precision(2);

	std::cerr << "Overall: " << clock.get(OVERALL) << "s" << std::endl;
	std::cerr << " OpenCL: " << clock.get(DEVICES) << "s" << std::endl;
	std::cerr << " AllocC:  " << clock.get(ALLOC_C)/O << "%" << std::endl;
	std::cerr << " FreeC:   " << clock.get(FREE_C)/O << "%" << std::endl;
}

} } // namespace map::detail
