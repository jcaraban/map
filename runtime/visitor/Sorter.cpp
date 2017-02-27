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
	prev_hash.clear();
}

void Sorter::static_visit(Node *node) {
	//
	std::unordered_set<Node*> uset;
	for (auto prev : node->prevList()) {
		if (prev->id > node->id && dynamic_cast<Feedback*>(prev))
			continue; // skip cyclic edges
		assert(prev_hash.find(prev) != prev_hash.end());
		uset.insert(prev_hash[prev].begin(),prev_hash[prev].end());
		uset.insert(prev);
	}
	prev_hash.insert( {node,uset} );
}

NodeList Sorter::sort(NodeList list) {
	clear();

	// Creates 'hashes of previous' to speed up the sorting
	for (auto node : list)
		static_visit(node);

	// lambda predicate givent to std::sort
	auto pred_dep_id = [&](Node *lhs, Node *rhs) {
		auto &ruset = prev_hash.find(rhs)->second;
		bool l_prev_r = (ruset.find (lhs) != ruset.end());

		auto &luset = prev_hash.find(lhs)->second;
		bool r_prev_l = (luset.find (rhs) != luset.end());

		if (l_prev_r) // left is previous to right
			return true;
		else if (r_prev_l) // right is previous to left
			return false; 
		else // no data dependencie, id decides order
			return lhs->id < rhs->id;
	};

	// Sorts by dependency 1st, by id 2nd
	node_list = list;
	std::sort(node_list.begin(),node_list.end(),pred_dep_id);

	return node_list;
}

#define DEFINE_VISIT(class) \
	void Sorter::visit(class *node) { \
		helper<class>(node); \
	}
	
	//DEFINE_VISIT(...)
#undef DEFINE_VISIT

} } // namespace map::detail
