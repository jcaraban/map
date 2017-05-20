/**
 * @file	Lister.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Lister.hpp"
#include <algorithm>


namespace map { namespace detail {

Lister::Lister()
	: node_list()
{ }

void Lister::clear() {
	visited.clear();
	queue = decltype(queue)();
	node_list.clear();
}

NodeList Lister::list(Node *node) {
	return list( NodeList(1,node) );
}

NodeList Lister::list(NodeList few_nodes) {
	clear();
	for (auto node : few_nodes) {
		queue.push(node);
	}
	while (not queue.empty()) {
		Node *node = queue.front();
		queue.pop();

		if (wasVisited(node))
			continue;
		setVisited(node);

		node_list.push_back(node); // Push into output

		for (auto forw : node->forwList())
			queue.push(forw);
		for (auto prev : node->prevList())
			queue.push(prev);
	}
	std::reverse(node_list.begin(),node_list.end());
	return node_list;
}

} } // namespace map::detail
