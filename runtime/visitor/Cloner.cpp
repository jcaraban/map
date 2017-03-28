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
	// Clones 'old' using 'virtual clone()' and the 'prev'/'back' clones
	Node *node = old->clone(old_hash);

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
