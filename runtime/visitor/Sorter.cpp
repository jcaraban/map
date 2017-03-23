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

		if (count == 0) { // ready nodes with 'prev = 0' go into the queue
			queue.push(node);
		} else { // nodes with 'prev > 0' store the count until 'prev = 0'
			prev_count[node] = count;
		}
	}

	// (1) 'queue' for the bfs topo-sort, (2) 'priority' for the id-sort
	while (not queue.empty()) {
		Node *node = queue.top();
		queue.pop();

		node_list.push_back(node);

		for (auto next : node->nextList()) {
			assert(prev_count.find(next) != prev_count.end());

			prev_count[next]--;
			if (prev_count[next] == 0) {
				prev_count.erase(next);
				queue.push(next);
			}
		}	
	}

	assert(prev_count.empty());
	return node_list;
}

#define DEFINE_VISIT(class) \
	void Sorter::visit(class *node) { \
		helper<class>(node); \
	}
	
	//DEFINE_VISIT(...)
#undef DEFINE_VISIT

} } // namespace map::detail
