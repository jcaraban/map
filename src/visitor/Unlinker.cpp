/**
 * @file	Unlinker.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Unlinker.hpp"
#include <algorithm>


namespace map { namespace detail {

Unlinker::Unlinker() { }

void Unlinker::clear() {
	visited.clear();
	isolated.clear();
	liberated.clear();
}

bool Unlinker::isIsolated(Node *node) {
	return isolated.find(node) != isolated.end();
}

void Unlinker::setIsolated(Node *node) {
	isolated.insert(node);
}

bool Unlinker::isLiberated(Node *node) {
	return liberated.find(node) != liberated.end();
}

void Unlinker::setLiberated(Node *node) {
	isolated.erase(node);
	liberated.insert(node);
}

NodeList Unlinker::unlink(const OwnerNodeList &node_list) {
	clear();
	NodeList ret_list;
	// In reverse order, ref=0 nodes are unlinked, which frees new nodes
	for (auto it=node_list.rbegin(); it!=node_list.rend(); it++) {
		Node *node = it->get();
		// Skip referred nodes and check for isolation
		if (node->ref > 0) {
			int count = node->ref - node->backList().size();
			for (auto next : node->nextList())
				count -= isIsolated(next);
			if (count == 0) // Mark it as isolated, but
				setIsolated(node); // it might be liberated
			continue;
		}
		// Inform 'prev' nodes
		for (auto &prev : node->prevList())
			prev->removeNext(node);
		// Inform 'forw' nodes
		for (auto &forw : node->forwList())
			forw->removeBack(node);
		// Register unlinked node
		ret_list.push_back(node);
	}
	// Walk Merge nodes to liberate the isolated
	for (auto it=node_list.rbegin(); it!=node_list.rend(); it++) {
		Node *node = it->get();
		if (node->pattern().is(MERGE) || node->pattern().is(SWITCH))
			node->accept(this);
	}
	// Second round, unlinks the liberated nodes
	for (auto it=node_list.rbegin(); it!=node_list.rend(); it++) {
		Node *node = it->get();
		// Only liberated nodes
		if (not isLiberated(node) || node->ref > 0)
			continue;
		// Inform 'prev' nodes
		for (auto &prev : node->prevList()) {
			prev->removeNext(node);
			setLiberated(prev);
		}
		// Register unlinked node
		ret_list.push_back(node);
	}
	
	return ret_list;
}

void Unlinker::visit(Merge *merge) {
	// Only isolated and loop-related Merge nodes (i.e. with forw/back edge)
	if (isIsolated(merge) && not merge->forw_list.empty()) {
		bool all_iso = true;
		for (auto next : merge->nextList())
			all_iso = all_iso && isIsolated(next);
		if (all_iso && isIsolated(merge->right())) {
			merge->right()->removeBack(merge);
			setLiberated(merge->right());
		}
	}
}

void Unlinker::visit(Switch *swit) {
	// Clear the unlinked 'true' / 'false' nodes from their lists
	auto pred = [&](Node *node) { return not is_included(node,swit->next_list); };
	auto &next_true = swit->next_true, &next_false = swit->next_false;
	next_true.erase(std::remove_if(next_true.begin(),next_true.end(),pred),next_true.end());
	next_false.erase(std::remove_if(next_false.begin(),next_false.end(),pred),next_false.end());
}

} } // namespace map::detail
