/**
 * @file	Fusioner.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: pipe-gently is not lossless, it could hide the optimal fusion when ignoring the data type (e.g. B8)
 * Note: pipeFusing clusters may over-qualify the pattern that one cluster sees of another (see drawings)
 * Note: sorting has to go after linking or will break Radial (out cl_mem arguments are moved if sorted)
 *
 * TODO: revise Bottom-Up approach. What about overlapping clusters?
 * TODO: revise the fusion of scalar... avoid creating false dependencies
 */

#include "Fusioner.hpp"
#include "../Runtime.hpp"
#include <algorithm>
#include <map>


namespace map { namespace detail {

/***********
   Methods
 ***********/

Fusioner::Fusioner(OwnerClusterList& cluster_list)
	: cluster_list(cluster_list)
{ }

void Fusioner::clear() {
	visited.clear();
	cluster_list.clear();
	cluster_list_of.clear();
	sorted_cluster_list.clear();
}

void Fusioner::fuse(NodeList list) {
	TimedRegion region(Runtime::getClock(),FUSION);
	clear();

	// ## 1st fusion stage ## fuse gently, w/o compromises

	for (auto node : list) { // Creates clusters and pipe-fuses gently 
		process(node);
		pipeGently(node);
	}

//print(cluster_list); // @

	for (auto node : list) { // Flat-fuses gently
		flatGently(node);
	}

//print(cluster_list); // @

	for (auto node : list) {
		processLoop(node); // Fuses loop head and tail nodes
	}

//print(cluster_list); // @

	// ## 2nd fusion stage ## fuse greedy, more aggresively

	for (auto it=list.rbegin(); it!=list.rend(); it++) {
		assert(cluster_list_of[*it].size() == 1);
		processBU(cluster_list_of[*it].front()); // Goes up cluster by cluster ## 2nd fusion stage ##
	}

//print(cluster_list); // @

	for (auto it=list.rbegin(); it!=list.rend(); it++) {		
		processScalar(cluster_list_of[*it].front());  // Flat-fuses lonely scalars
	}

//print(cluster_list); // @

	for (auto it=list.rbegin(); it!=list.rend(); it++) {		
		process(cluster_list_of[*it].front()); // flat-fuse
	}

//print(cluster_list); // @

	// ## 3rd fusion stage ## forwards D0-FREE, links dependent clusters, sorts

	auto free = [](Node *n){ return n->pattern()==INPUT || n->pattern()==FREE; };
	forwarding(free); // Replicates lonely free nodes ## 3rd fusion tage ##

//print(cluster_list); // @

	linking(); // Marks as input/output all those nodes in the cluster boundary

	// fuse the output nodes only at the end

	sorting(); // Sorts 'cluster_list' (in topological order) and cluster_list[*]->node_list (in id order)

	print(sorted_cluster_list); // Prints clusters and nodes once linked & sorted
}

Cluster* Fusioner::newCluster() {
	return Runtime::getInstance().addCluster( new Cluster() );
}

void Fusioner::removeCluster(Cluster *cluster) {
	auto pred = [&](const std::unique_ptr<Cluster> &g) { return g.get() == cluster; };
	cluster_list.erase(std::remove_if(cluster_list.begin(),cluster_list.end(),pred),cluster_list.end());
}

bool Fusioner::canPipeFuse(Cluster *top, Cluster *bot) {
	if (top == bot)
		return false; // same node
	if (not is_included(bot,top->nextList()))
		assert(false); // no direct pipe-relation
	for (auto next : top->nextList())
		if (next != bot && next->isNext(bot))
			return false; // found cycle

	bool pipe = detail::canPipeFuse(bot->prevPattern(top),top->nextPattern(bot));
	bool flat = detail::canFlatFuse(top->pattern(),bot->pattern());
	bool inum = full_unique_join(top->inputList(),bot->inputList()).size() <= Runtime::getConfig().max_in_block;
	bool onum = full_unique_join(top->outputList(),bot->outputList()).size() <= Runtime::getConfig().max_out_block;

	return pipe && flat && inum && onum;
}

bool Fusioner::canFlatFuse(Cluster *left, Cluster *right) {
	if (left == right)
		return false; // same node
	if (left->isNext(right) || right->isNext(left))
		return false; // found cycle or pipe-relation

	bool flat = detail::canFlatFuse(left->pattern(),right->pattern());
	bool inum = full_unique_join(left->inputList(),right->inputList()).size() <= Runtime::getConfig().max_in_block;
	bool onum = full_unique_join(left->outputList(),right->outputList()).size() <= Runtime::getConfig().max_out_block;
	
	return flat && inum && onum;
}

Cluster* Fusioner::pipeFuseCluster(Cluster *&top, Cluster *&bot) {
	assert(canPipeFuse(top,bot));
	
	// Moves 'bot' nodes to 'top'
	for (auto node : bot->nodeList()) {
		top->addNode(node);
		remove_value(bot,cluster_list_of[node]);
		cluster_list_of[node].push_back(top);
	}
	// Moves 'bot' input-nodes to 'top'
	for (auto node : bot->inputList()) {
		top->addInputNode(node);
		remove_value(bot,cluster_list_of[node]);
		cluster_list_of[node].push_back(top);
	}
	// Moves 'bot' output-nodes to 'top'
	for (auto node : bot->outputList()) {
		top->addOutputNode(node);
		remove_value(bot,cluster_list_of[node]);
		cluster_list_of[node].push_back(top);
	}
	
	// Moves 'bot' prev-clusters to 'top'
	for (auto prev : bot->prevList()) {
		if (prev == top)
			continue;

		// Adds 'top' as next-cluster of 'prev'
		prev->addNext(top,prev->nextPattern(bot));

		// Adds 'prev' as prev-cluster of 'top'
		top->addPrev(prev,bot->prevPattern(prev));

		// 'prev' no longer points to 'bot'
		prev->removeNext(bot);
	}

	// Moves 'bot' next-clusters to 'top'
	for (auto next : bot->nextList()) {
		assert(next != top); // Otherwise there was a cycle

		// Adds 'top' as prev-cluster of 'next', (accumulates both patterns)
		next->addPrev(top,next->prevPattern(bot)+bot->prevPattern(top));
		// @ next-to-top pattern is a worst-case

		// Adds 'next' as next-cluster of 'top'
		top->addNext(next,bot->nextPattern(next));

		// 'next' no longer points to 'bot'
		next->removePrev(bot);
	}

	for (auto back : bot->backList()) { // @
		back->addForw(top);
		top->addBack(back);
		back->removeForw(bot);
	}

	for (auto forw : bot->forwList()) { // @
		forw->addBack(top);
		top->addForw(forw);
		forw->removeBack(bot);
	}

	// No need to touch 'top' next-clusters

	// Updates 'top' prev-clusters with 'bot' pattern
	for (auto prev : top->prevList()) {
		prev->addNext(top,top->nextPattern(bot));
		// @ prev-to-top pattern is a worst-case
	}

	// 'top' no longer points to 'bot'
	top->removeNext(bot);

	// Pipe-fuses 'top' to 'bot' pattern
	top->pattern() += bot->pattern();

	// Finally erases 'bot' from the main list of clusters
	removeCluster(bot);

	Cluster *ret = top;
	top = nullptr;
	bot = nullptr;
	return ret;
}

Cluster* Fusioner::flatFuseCluster(Cluster *&left, Cluster *&right) {
	assert(canFlatFuse(left,right));
	
	// Moves 'right' nodes to 'left'
	for (auto node : right->nodeList()) {
		left->addNode(node);
		remove_value(right,cluster_list_of[node]);
		cluster_list_of[node].push_back(left);
	}
	// Moves 'right' input-nodes to 'left'
	for (auto node : right->inputList()) {
		left->addInputNode(node);
		remove_value(right,cluster_list_of[node]);
		cluster_list_of[node].push_back(left);
	}
	// Moves 'right' output-nodes to 'left'
	for (auto node : right->outputList()) {
		left->addOutputNode(node);
		remove_value(right,cluster_list_of[node]);
		cluster_list_of[node].push_back(left);
	}
	
	// Moves 'right' prev-clusters to 'left'
	for (auto prev : right->prevList()) {
		assert(prev != left); // no pipe-relation

		// Adds 'left' as next-cluster of 'prev'
		prev->addNext(left,prev->nextPattern(right));

		// Adds 'prev' as prev-cluster of 'left'
		left->addPrev(prev,right->prevPattern(prev));

		// 'prev' no longer points to 'right'
		prev->removeNext(right);
	}

	// Moves 'right' next-clusters to 'left'
	for (auto next : right->nextList()) {
		assert(next != left); // no pipe-relation

		// Adds 'left' as prev-cluster of 'next'
		next->addPrev(left,next->prevPattern(right));

		// Adds 'next' as next-cluster of 'left'
		left->addNext(next,right->nextPattern(next));

		// 'next' no longer points to 'right'
		next->removePrev(right);
	}

	for (auto back : right->backList()) { // @
		back->addForw(left);
		left->addBack(back);
		back->removeForw(right);
	}

	for (auto forw : right->forwList()) { // @
		forw->addBack(left);
		left->addForw(forw);
		forw->removeBack(right);
	}

	// flat-fuses 'left' with 'right' pattern
	left->pattern() += right->pattern();

	// Finally erases 'right' from the main list of clusters
	removeCluster(right);
	
	Cluster *ret = left;
	left = nullptr;
	right = nullptr;
	return ret;
}

Cluster* Fusioner::freeFuseCluster(Cluster *&one, Cluster *&other) {
	if (one == other)
		return one;
	for (auto next : other->nextList())
		if (next->isNext(one))
			assert(0); // found cycle
	
	// Moves 'other' nodes to 'one'
	for (auto node : other->nodeList()) {
		one->addNode(node);
		remove_value(other,cluster_list_of[node]);
		cluster_list_of[node].push_back(one);
	}
	// Moves 'other' input-nodes to 'one'
	for (auto node : other->inputList()) {
		one->addInputNode(node);
		remove_value(other,cluster_list_of[node]);
		cluster_list_of[node].push_back(one);
	}
	// Moves 'other' output-nodes to 'one'
	for (auto node : other->outputList()) {
		one->addOutputNode(node);
		remove_value(other,cluster_list_of[node]);
		cluster_list_of[node].push_back(one);
	}
	
	// Moves 'other' prev-clusters to 'one'
	for (auto prev : other->prevList()) {
		if (prev == one)
			continue;

		// Adds 'one' as next-cluster of 'prev'
		prev->addNext(one,prev->nextPattern(other));

		// Adds 'prev' as prev-cluster of 'one'
		one->addPrev(prev,other->prevPattern(prev));

		// 'prev' no longer points to 'other'
		prev->removeNext(other);
	}

	// Moves 'other' next-clusters to 'one'
	for (auto next : other->nextList()) {
		if (next == one)
			continue;

		// Adds 'one' as prev-cluster of 'next'
		next->addPrev(one,next->prevPattern(other));

		// Adds 'next' as next-cluster of 'one'
		one->addNext(next,other->nextPattern(next));

		// 'next' no longer points to 'other'
		next->removePrev(other);
	}

	for (auto back : other->backList()) { // @
		back->addForw(one);
		one->addBack(back);
		back->removeForw(other);
	}

	for (auto forw : other->forwList()) { // @
		forw->addBack(one);
		one->addForw(forw);
		forw->removeBack(other);
	}

	// Updates 'one' prev-clusters with 'other' pattern
	if (is_included(other,one->nextList())) {
		for (auto prev : one->prevList()) {
			prev->addNext(one,one->nextPattern(other));
		}
		// 'one' no longer points to 'other'
		one->removeNext(other);
	}
	
	// Updates 'one' next-clusters with 'other' pattern
	if (is_included(other,one->prevList())) {
		for (auto next : one->nextList()) {
			next->addPrev(one,one->prevPattern(other));
		}
		// 'one' no longer points to 'other'
		one->removePrev(other);
	}

	// Fuses 'one' with 'other' pattern
	one->pattern() += other->pattern();

	// Finally erases 'other' from the main list of clusters
	removeCluster(other);
	
	Cluster *ret = one;
	one = nullptr;
	other = nullptr;
	return ret;
}

void Fusioner::process(Node *node) {
	Cluster *new_cluster = newCluster(); // Creates a new cluster for the node
	new_cluster->addAutoNode(node); // Adds node to the new cluster
	cluster_list_of[node].push_back(new_cluster); // Adds the new cluster to the cluster list of node

	for (auto prev : node->prevList()) {
		Cluster *prev_cluster = cluster_list_of[prev].front(); // Nodes have max. 1 cluster at this point
		prev_cluster->addNext(new_cluster,new_cluster->pattern()); // Giving the pattern this way works because
		new_cluster->addPrev(prev_cluster,prev_cluster->pattern()); // only LOCAL / FREE patters are fused now
	}

	for (auto back : node->backList()) { // @
		Cluster *back_cluster = cluster_list_of[back].front();
		back_cluster->addForw(new_cluster);
		new_cluster->addBack(back_cluster);
	}
}

void Fusioner::pipeGently(Node *node) {
	if (not Runtime::getConfig().code_fusion)
		return;
	auto isFreeOrLocal = [](Cluster *cluster) { return cluster->pattern().is(FREE) || cluster->pattern().is(LOCAL); };
	Cluster *new_cluster = cluster_list_of[node].front();

	int i = 0;
	while (i < new_cluster->prevList().size()) {
		Cluster *prev_cluster = new_cluster->prevList()[i];
		i++;
		bool fuse_free = isFreeOrLocal(new_cluster) && isFreeOrLocal(prev_cluster);
		bool fuse_d0dn = not (prev_cluster->numdim() == D0 && new_cluster->numdim() != D0 && prev_cluster->pattern() != FREE);
		bool fuse_dnd0 = not (prev_cluster->numdim() != D0 && new_cluster->numdim() == D0);
		bool fuse_next = prev_cluster->nextList().size() == 1;

		if (fuse_free && fuse_d0dn && fuse_dnd0 && fuse_next && canPipeFuse(prev_cluster,new_cluster)) {
			new_cluster = pipeFuseCluster(prev_cluster,new_cluster);
			i = 0; // rather than resetting, could be improved with a queue
		}
	}
}

void Fusioner::flatGently(Node *node) {
	if (not Runtime::getConfig().code_fusion)
		return;
	auto isFreeOrLocal = [](Cluster *cluster) { return cluster->pattern().is(FREE) || cluster->pattern().is(LOCAL); };
	Cluster *node_cluster = cluster_list_of[node].front();
	
	if (node_cluster->nextList().size() < 2)
		return; // Nothing to flat-fuse here

	int i = 0;
	while (i < node->nextList().size())
	{
		Node *left = node->nextList()[i++];
		Cluster *left_cluster = cluster_list_of[left].front();

		bool free_local = isFreeOrLocal(left_cluster);
		bool dim_equal = node_cluster->numdim() == left_cluster->numdim();

		if (not free_local || not dim_equal || left_cluster == node_cluster)
			continue;

		int j = i; // i was already incremented
		while (j < node->nextList().size())
		{
			Node *right = node->nextList()[j++];
			Cluster *right_cluster = cluster_list_of[right].front();

			bool free_local = isFreeOrLocal(right_cluster);
			bool dim_equal = node_cluster->numdim() == right_cluster->numdim();

			if (not free_local || not dim_equal || right_cluster == node_cluster || right_cluster == left_cluster)
				continue;

			if (canFlatFuse(left_cluster,right_cluster)) {
				left_cluster = flatFuseCluster(left_cluster,right_cluster);
			}
		}
	}
}

void Fusioner::processLoop(Node *node) {
	if (node->pattern().isNot(HEAD) && node->pattern().isNot(TAIL))
		return; // Only 'heads' and 'tails' now

	Node *mark = nullptr; // marks the cluster to fuse with

	if (auto cast = dynamic_cast<LoopHead*>(node)) {
		mark = cast->loop()->headList()[0];
	} else if (auto cast = dynamic_cast<LoopTail*>(node)) {
		mark = cast->loop()->tailList()[0];
	} else {
		assert(0);
	}

	assert(cluster_list_of.find(node)->second.size() == 1);
	assert(cluster_list_of.find(mark)->second.size() == 1);

	Cluster *node_cluster = cluster_list_of.find(node)->second.front();
	Cluster *mark_cluster = cluster_list_of.find(mark)->second.front();
	mark_cluster = freeFuseCluster(mark_cluster,node_cluster);
}

void Fusioner::processScalar(Cluster *cluster) {
	if (not Runtime::getConfig().code_fusion)
		return;

	auto isFreeOrLocal = [](Cluster *cluster) { return cluster->pattern().is(FREE) || cluster->pattern().is(LOCAL); };

	// Next() direction
	int i = 0;
	while (i < cluster->nextList().size())
	{
		Cluster *left_cluster = cluster->nextList()[i++];

		bool free_local = isFreeOrLocal(left_cluster);
		bool dim_d0 = left_cluster->numdim() == D0;
		bool diff_cluster = left_cluster != cluster;

		if (not free_local || not dim_d0 || not diff_cluster)
			continue;

		int j = i; // i was already incremented
		while (j < cluster->nextList().size())
		{
			Cluster *right_cluster = cluster->nextList()[j++];

			bool free_local = isFreeOrLocal(right_cluster);
			bool dim_d0 = right_cluster->numdim() == D0;
			bool diff_cluster = right_cluster != cluster && right_cluster != left_cluster;

			if (not free_local || not dim_d0 || not diff_cluster)
				continue;

			if (canFlatFuse(left_cluster,right_cluster)) {
				left_cluster = flatFuseCluster(left_cluster,right_cluster);
			}
		}
	}

	// Prev() direction
	i = 0;
	while (i < cluster->prevList().size())
	{
		Cluster *left_cluster = cluster->prevList()[i++];

		bool free_local = isFreeOrLocal(left_cluster);
		bool dim_d0 = left_cluster->numdim() == D0;
		bool diff_cluster = left_cluster != cluster;

		if (not free_local || not dim_d0 || not diff_cluster)
			continue;

		int j = i; // i was already incremented
		while (j < cluster->prevList().size())
		{
			Cluster *right_cluster = cluster->prevList()[j++];

			bool free_local = isFreeOrLocal(right_cluster);
			bool dim_d0 = right_cluster->numdim() == D0;
			bool diff_cluster = right_cluster != cluster && right_cluster != left_cluster;

			if (not free_local || not dim_d0 || not diff_cluster)
				continue;

			if (canFlatFuse(left_cluster,right_cluster)) {
				left_cluster = flatFuseCluster(left_cluster,right_cluster);
			}
		}
	}
}

void Fusioner::process(Cluster *cluster) {
	if (!Runtime::getConfig().code_fusion)
		return;

	if (cluster->pattern().is(MERGE))
		return; // 'next' of merge must not flat-fuse

	//// Flat-fusion
	int i = 0;
	while (i < cluster->nextList().size())
	{
		int j = i+1;
		while (j < cluster->nextList().size())
		{
			Cluster *left = cluster->nextList()[i];
			Cluster *right = cluster->nextList()[j];
			// @
			bool common_input = false;
			for (auto node : cluster->nodeList())
				for (auto next : node->nextList())
					if (is_included(next,left->nodeList()))
						for (auto next : node->nextList())
							if (is_included(next,right->nodeList()))
								common_input = true;
			//
			if (common_input && canFlatFuse(left,right)) {
				left = flatFuseCluster(left,right);
				i = j = 0; // reset
			} else {
				j++;
			}
		}
		i++;
	}
}

void Fusioner::processBU(Cluster *cluster) { // @
	if (!Runtime::getConfig().code_fusion)
		return;
	
	if (visited.find(cluster) != visited.end())
		return;
	visited.insert(cluster);

	//// Pipe-fusion
	int i = 0;
	while (i < cluster->prevList().size())
	{
		Cluster *bot = cluster;
		Cluster *top = cluster->prevList()[i++];
		bool d0dn = not (top->numdim() == D0 && bot->numdim() != D0
					&& top->pattern() != FREE && top->pattern().isNot(MERGE));
		bool dnd0 = not (top->numdim() != D0 && bot->numdim() == D0
					&& bot->pattern().isNot(ZONAL) && bot->pattern().isNot(STATS) && bot->pattern().isNot(MERGE));

		if (d0dn && dnd0 && canPipeFuse(top,bot)) {
			cluster = pipeFuseCluster(top,bot);
			i = 0; // reset
		}
	}

	//// Going up
	int s = cluster->prevList().size();
	i = 0;
	while (i < s) { 
		processBU(cluster->prevList()[i++]);
		if (s != cluster->prevList().size()) {
			s = cluster->prevList().size();
			i = 0;
		}
	}
}

void Fusioner::forwarding(std::function<bool(Node*)> for_pred) {
	std::map<std::pair<Cluster*,Cluster*>,std::vector<Node*>> forward;
	auto all_nodes = [](Cluster *cluster) { // @ change to full_unique_join ?
		auto in_body = full_join(cluster->inputList(),cluster->nodeList());
		return full_unique_join(in_body,cluster->outputList());
	};

	// Node forwarding phase

	for (auto &i : cluster_list) { // For every 'cluster' in cluster_list...
		Cluster *cluster = i.get();

		for (auto &node : all_nodes(cluster)) { // For every 'node' in 'cluster'
			if (!for_pred(node))
				continue; // Which satisfy the given predicate
			bool forwarded = false;

			for (auto next_node : node->nextList()) { // For every 'next-node' of 'node'
				for (auto next_cluster : cluster_list_of[next_node]) { // For every cluster (aka 'next-cluster') of 'next-node'
					if (!is_included(node,all_nodes(next_cluster))) // If 'node' is not included in 'next-cluster'
					{
						next_cluster->addAutoNode(node); // Forward 'node' into 'next-cluster'
						cluster_list_of[node].push_back(next_cluster); // Adds 'next-cluster' into 'cluster_list' of 'node'
						forwarded = true;
						forward[{cluster,next_cluster}].push_back(node);
					}
				}
			}

			if (forwarded) // If 'node' was forwarded
			{
				auto pred = [&](Node *n) { return !is_included(n,cluster->nodeList()); };
				if (std::all_of(node->nextList().begin(),node->nextList().end(),pred)) {
					cluster->removeAutoNode(node); // If all nexts of 'node' are outside 'cluster', removes 'node'
					remove_value(cluster,cluster_list_of[node]); // Removes 'cluster' from 'cluster_list' of 'node'
				}
			}
		}
	}

	// Cluster-to-cluster unlinking phase

	for (auto f : forward) { // For every link 'cluster'<->'next-cluster'
		Cluster *cluster = f.first.first, *next_cluster = f.first.second;
		bool linked = false;
		for (auto node : left_join(cluster->nodeList(),f.second))
			if (node->pattern() != FREE)
				for (auto next_node : node->nextList())
					if (is_included(next_node,next_cluster->nodeList()))
						linked = true;
		if (!linked) { // If there remains no links between nodes, remove clusters link
			cluster->removeNext(next_cluster);
			next_cluster->removePrev(cluster);
		}
	}

	// Cluster removing phase
	auto rm_pred = [&](const std::unique_ptr<Cluster> &g) { return all_nodes(g.get()).empty(); };
	cluster_list.erase(std::remove_if(cluster_list.begin(),cluster_list.end(),rm_pred),cluster_list.end());
}

void Fusioner::linking() {
	// For cluster, node, next-node, next-cluster: if node !€ in next-cluster, node becomes in/out-node
	auto next_back_list_of = [&](Node *node) { return full_join(node->nextList(),node->backList()); };

	for (auto &i : cluster_list) { // For every 'cluster' in cluster_list...
		Cluster *cluster = i.get();
		//assert(!cluster->nodeList().empty());

		for (auto &node : cluster->nodeList()) // For every 'node' in 'cluster'
		{
			for (auto next_node : next_back_list_of(node)) // For every 'next'/'back' of 'node'
			{
				for (auto next_cluster : cluster_list_of[next_node]) // For every cluster (aka 'next-cluster') of 'next-node'
				{
					if (!is_included(node,next_cluster->nodeList())) // If 'node' is not included in 'next-cluster'
					{
						cluster->addOutputNode(node); // 'node' becomes an output of its 'cluster'
						next_cluster->addInputNode(node); // 'next_cluster's accept 'node' as an input
					}
				}
			}
		}
	}

	// Some patterns require their nodes to become output of their cluster

	for (auto &i : cluster_list) {  // For every 'cluster' in cluster_list...
		Cluster *cluster = i.get();
		for (auto &node : cluster->nodeList()) { // For every 'node' in 'cluster'
			Pattern pat = node->pattern();
			if (pat.is(ZONAL) || pat.is(RADIAL) || pat.is(SPREAD) || pat.is(STATS) || pat.is(LOOP)) {
				cluster->addOutputNode(node); // 'node' becomes an output of its 'cluster'
			}
		}
	}

	// Moves D0-FREE nodes from the node_list to the input_list, improves the reutilization rate of compiled kernels
	/*
	for (auto &g : cluster_list) { // For every 'cluster' in cluster_list...
		Cluster *cluster = g.get();
		int i = 0;
		while (i < cluster->nodeList().size()) { // For every 'node' in 'cluster'
			Node *node = cluster->nodeList()[i];
			if (node->numdim() == D0 && node->pattern() == FREE) { // Which is D0 and FREE
				cluster->removeNode(node); // Removes from nodeList()
				cluster->addInputNode(node); // Inserts into inputList()
			} else {
				i++;
			}
		}
	}
	*/
}

void Fusioner::sorting() {
	ClusterList unsort_cluster_list;
	for (auto &cluster : cluster_list)
		unsort_cluster_list.push_back(cluster.get());

	// For each cluster, sorts its lists of nodes by Node id
	for (auto cluster : unsort_cluster_list) {
		std::sort(cluster->node_list.begin(),cluster->node_list.end(),node_id_less());
		std::sort(cluster->in_list.begin(),cluster->in_list.end(),node_id_less());
		std::sort(cluster->out_list.begin(),cluster->out_list.end(),node_id_less());
	}

	// Topological sort in order of dependencies and first-node id
	sorted_cluster_list = toposort(unsort_cluster_list);

	// Numerates the clusters with an unique 'id' after sorting
	int id_count = 0;
	for (auto cluster : sorted_cluster_list) {
		cluster->id = id_count++;
	}

	// @ It would be good to sort the cluster prev_list and next_list,
	// but prev_pat and next_pat must be ordered accordingly
}

ClusterList Fusioner::toposort(ClusterList list) {
	struct greater {
		bool operator()(Cluster *ca, Cluster *cb){
			Node *na = not ca->nodeList().empty() ? ca->nodeList().front() : ca->outputList().front();
			Node *nb = not cb->nodeList().empty() ? cb->nodeList().front() : cb->outputList().front();
			return not (ca->isNext(cb) ? true : ca->isPrev(cb) ? false : na->id < nb->id);
		}
	};

	std::priority_queue<Cluster*,std::vector<Cluster*>,greater> prique;
	std::unordered_map<Cluster*,int> prev_count; //!< Keeps the count of remaining prevs
	ClusterList cluster_list;

	assert(not list.empty());
	cluster_list.reserve(list.size());

	// Walks all clusters first to registers them in the data structures
	for (auto cluster : list) {
		int count = cluster->prevList().size();
		prev_count.insert({cluster,count});
		
		if (count == 0) { // ready clusters with 'prev = 0' go into the 'prique'
			prique.push(cluster);
			prev_count.erase(cluster);
		}
	}

	// (1) 'queue' for the bfs topo-sort, (2) 'priority' for the id-sort
	while (not prique.empty()) {
		Cluster *cluster = prique.top();
		prique.pop();

		cluster_list.push_back(cluster); // Push into output

		for (auto next : cluster->nextList()) {
			//if (prev_count.find(next) == prev_count.end())
			//	continue; // not included in 'list'

			auto it = prev_count.find(next);
			assert(it != prev_count.end());
			auto &count = it->second;

			count--;
			if (count == 0) {
				prique.push(next);
				prev_count.erase(next);
			}
		}	
	}

	assert(prev_count.empty());
	return cluster_list;
}

void Fusioner::print(OwnerClusterList &list) {
	ClusterList non_owner;
	for (auto &cluster : list)
		non_owner.push_back(cluster.get());
	print(non_owner);
}

void Fusioner::print(ClusterList &list) {
	for (auto i : list) {
		std::cout << i->pattern() << "  " << i << std::endl;
		for (auto j : i->inputList())
			std::cout << "    " << j->shortName() << ", " << j->numdim() << " : " << j->id << std::endl;
		std::cout << "    --" << std::endl;
		for (auto j : i->nodeList())
			std::cout << "    " << j->longName() << ", " << j->numdim() << " : " << j->id << std::endl;
		std::cout << "    --" << std::endl;
		for (auto j : i->outputList())
			std::cout << "    " << j->shortName() << ", " << j->numdim() << " : " << j->id << std::endl;
		std::cout << "  prev:" << std::endl;
		for (auto j=i->prevList().begin(); j!=i->prevList().end(); j++)
			std::cout << "    " << (*j) << " " << i->prevPattern(j) << std::endl;
		std::cout << "  next:" << std::endl;
		for (auto j=i->nextList().begin(); j!=i->nextList().end(); j++)
			std::cout << "    " << (*j) << " " << i->nextPattern(j) << std::endl;
		std::cout << "  back:" << std::endl;
		for (auto j=i->backList().begin(); j!=i->backList().end(); j++)
			std::cout << "    " << (*j) << " " << Pattern(NONE_PATTERN) << std::endl;
		std::cout << "  forw:" << std::endl;
		for (auto j=i->forwList().begin(); j!=i->forwList().end(); j++)
			std::cout << "    " << (*j) << " " << Pattern(NONE_PATTERN) << std::endl;
		std::cout << std::endl;
	}
	std::cout << "--------------------" << std::endl;
}

} } // namespace map::detail
