/**
 * @file	Lister.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: Im sure there must be some more performant alternatives to isoprev/next,
 *       but this was the easiest way of handling the cycles caused by back-edges
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

	if (all_visited) // all 'node' dependencies are met and it is added to 'list'
	{
		node_list.push_back(node);
		setVisited(node);
		
		for (auto iso : isonext[node]) {
			//remove_value(node,isoprev[iso]);
			auto &list = isoprev[iso];
			assert( std::find(list.begin(),list.end(),node) != list.end() );
			list.erase(std::remove(list.begin(),list.end(),node),list.end());
			// @@
			if (isoprev[iso].empty())
				static_visit(iso);
		}
		isonext.erase(node);
	}
	else // marks 'prev' nodes isolating 'node'
	{
		for (auto prev : node->prevList()) {
			if (not wasVisited(prev)) {
				// @@
				auto &list = isonext[prev];
				if (std::find(list.begin(),list.end(),node) != list.end())
					continue;
				assert( std::find(list.begin(),list.end(),node) == list.end() );
				//
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
