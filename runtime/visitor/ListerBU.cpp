/**
 * @file	ListerBU.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "ListerBU.hpp"
#include <algorithm>


namespace map { namespace detail {

ListerBU::ListerBU()
	: node_list()
{ }

void ListerBU::clear() {
	visited.clear();
	node_list.clear();
}

void ListerBU::static_visit(Node *node) {
	if (wasVisited(node))
		return;
	setVisited(node);

	for (auto &prev : node->prevList())
		static_visit(prev);

	node_list.push_back(node);
}

NodeList ListerBU::list(Node *node) {
	clear();
	static_visit(node);
	return node_list;
}

NodeList ListerBU::list(NodeList few_nodes) {
	clear();
	for (auto node : few_nodes)
		static_visit(node);
	return node_list;
}

#define DEFINE_VISIT(class) \
	void ListerBU::visit(class *node) { \
		helper<class>(node); \
	}
	
	//DEFINE_VISIT(...)
#undef DEFINE_VISIT

} } // namespace map::detail
