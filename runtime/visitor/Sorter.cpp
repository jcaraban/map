/**
 * @file	Sorter.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: could the complexity of this algorithm be improved?
 */

#include "Sorter.hpp"
#include <algorithm>


namespace map { namespace detail {

Sorter::Sorter()
	: node_list()
{ }

void Sorter::clear() {
	//visited.clear();
	node_list.clear();
	prev_count.clear();
	queue = decltype(queue)();
}

void Sorter::static_visit(Node *node) {
	//
}

NodeList Sorter::sort(NodeList list) {
	clear();
	assert(not list.empty());

	// Walks all nodes first to registers them in the data structures
	for (auto node : list) {
		int count = node->prevList().size();
		// Feedback nodes are a exception...
		//auto feed = dynamic_cast<Feedback*>(node);
		//if (feed && feed->feedIn())
		//	count--;

		if (count == 0) { // ready nodes go into the queue
			queue.push(node);
		} else { // nodes with '> 0' prevs keep the count
			prev_count[node] = count;
		}
	}

	// (1) 'queue' for the bfs topo-sort, (2) 'priority' for the id-sort
	while (not queue.empty()) {
		Node *node = queue.top();
		queue.pop();

		node_list.push_back(node);

		for (auto next : node->nextList()) {
			// Feedback nodes are a exception...
			//auto feedout = dynamic_cast<Feedback*>(node);
			//auto feedin = dynamic_cast<Feedback*>(next);
			//if (feedin && feedout)
			//	continue;
			//assert(prev_count.find(next) != prev_count.end());

			prev_count[next]--;
			if (prev_count[next] == 0) {
				prev_count.erase(next);
				queue.push(next);
			}
		}	
	}

	//assert(prev_count.empty());
	return node_list;
}

#define DEFINE_VISIT(class) \
	void Sorter::visit(class *node) { \
		helper<class>(node); \
	}
	
	//DEFINE_VISIT(...)
#undef DEFINE_VISIT

} } // namespace map::detail
