/**
 * @file	Cloner.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Cloner.hpp"
#include <algorithm>


namespace map { namespace detail {

Cloner::Cloner(OwnerNodeList &list)
	: clone_list(list)
{ }

void Cloner::clear() {
	//visited.clear();
	clone_list.clear();
}

NodeList Cloner::clone(NodeList list) {
	clear();
	// Push the new nodes into 'clone_list'
	for (auto node : list)
		static_visit(node);
	// Copy the clones to 'ret_list'
	NodeList ret_list;
	for (auto &node : clone_list)
		ret_list.push_back(node.get());
	// Walk some special cases, like Switch
	for (auto node : ret_list)
		if (node->pattern().is(SWITCH))
			node->accept(this);
	return ret_list;
}

void Cloner::static_visit(Node *old) {
	// Clones 'old' using 'virtual clone()' and the 'prev'/'back' clones
	Node *node = old->clone(old_hash);

	// Pushes the clone to the list and sets the relation old<->new
	clone_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Switch *swit) {
	NodeList true_list = swit->next_true;
	NodeList false_list = swit->next_false;

	swit->next_true.clear();
	for (auto next : true_list)
		swit->addTrue(old_hash[next]);

	swit->next_false.clear();
	for (auto next : false_list)
		swit->addFalse(old_hash[next]);
}

} } // namespace map::detail
