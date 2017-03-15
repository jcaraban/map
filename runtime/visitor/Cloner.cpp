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

void Cloner::static_visit(Node *old) {
	NodeList new_prev_list;
	new_prev_list.reserve(old->prevList().size());
	// Fills the list of 'prev' nodes with the new clones
	for (auto prev : old->prevList())
		new_prev_list.push_back( old_hash.find(prev)->second );
	// Clones 'old' using 'virtual clone()' and the 'prev' clones
	Node *node = old->clone(new_prev_list);

	// ... continue ... ->clone() also needs the new_back_list?

	// Pushes the clone to the list and sets the relation old<->new
	clone_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

NodeList Cloner::clone(NodeList list) {
	clear();
	for (auto node : list)
		static_visit(node);
	NodeList ret_list;
	for (auto &node : clone_list)
		ret_list.push_back(node.get());
	return ret_list;
}

} } // namespace map::detail
