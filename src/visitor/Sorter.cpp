/**
 * @file	Sorter.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: inner_join can be accelerated with an std::unord_set of 'included nodes'
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
	prique = decltype(prique)();
}

NodeList Sorter::sort(NodeList list) {
	clear();
	assert(not list.empty());

	// Walks all nodes first to registers them in the data structures
	for (auto node : list) {
		int count = node->prevList().size();
		prev_count.insert({node,count});
		
		if (count == 0) { // ready nodes with 'prev = 0' go into the 'prique'
			prique.push(node);
			prev_count.erase(node);
		}
	}

	// (1) 'queue' for the bfs topo-sort, (2) 'priority' for the id-sort
	while (not prique.empty()) {
		Node *node = prique.top();
		prique.pop();

		node_list.push_back(node); // Push into output

		for (auto next : node->nextList()) {
			if (prev_count.find(next) == prev_count.end())
				continue; // not included in 'list'

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
	return node_list;
}

} } // namespace map::detail
