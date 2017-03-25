/**
 * @file	Lister.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: there might be some more performant alternatives to isoprev/next,
 *       but this is the easiest way of handling the cycles caused by Feedback
 */

#include "Lister.hpp"
#include <algorithm>


namespace map { namespace detail {

Lister::Lister()
	: node_list()
{ }

void Lister::clear() {
	visited.clear();
	marked.clear();
	isoprev.clear();
	isonext.clear();
	node_list.clear();
}

bool Lister::wasMarked(Node *node) {
	return marked.find(node) != marked.end();
}

void Lister::setMarked(Node *node) {
	marked.insert(node);
}

void Lister::unMarked(Node *node) {
	marked.erase(node);
}

void Lister::static_visit(Node *node) {
	//
	if (wasVisited(node) || wasMarked(node))
		return;
	setMarked(node);

	bool all_visited = true;
	for (auto prev : node->prevList()) {
		static_visit(prev); // goes up recursively...
		all_visited &= wasVisited(prev);
	}
	for (auto forw : node->forwList()) {
		static_visit(forw); // forward nodes depending on this, due to cycles
	}

	if (all_visited)
	{
		node_list.push_back(node);
		setVisited(node);

		for (auto iso : isonext[node]) {
			remove_value(node,isoprev[iso]);
			if (isoprev[iso].empty())
				static_visit(iso);
		}
	}
	else
	{
		for (auto prev : node->prevList()) {
			if (not wasVisited(prev)) {
				isonext[prev].push_back(node);
				isoprev[node].push_back(prev);
			}
		}
	}

	unMarked(node);
}

NodeList Lister::list(Node *node) {
	clear();
	static_visit(node);
	return node_list;
}

NodeList Lister::list(NodeList few_nodes) {
	clear();
	for (auto node : few_nodes)
		static_visit(node);
	return node_list;
}

#define DEFINE_VISIT(class) \
	void Lister::visit(class *node) { \
		helper<class>(node); \
	}
	
	//DEFINE_VISIT(...)
#undef DEFINE_VISIT

} } // namespace map::detail
