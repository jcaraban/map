/**
 * @file	Cloner.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor that clones the DAG (given as a list) into a new graph
 */

#ifndef MAP_RUNTIME_VISITOR_CLONER_HPP_
#define MAP_RUNTIME_VISITOR_CLONER_HPP_

#include "Visitor.hpp"
#include <unordered_map>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Return an ordered list with new nodes forming a cloned graph
 */
struct Cloner : public Visitor
{
  // constructor and main function
	Cloner(OwnerNodeList &list);
	NodeList clone(NodeList list);

  // methods
	void clear();

  // visit
	void static_visit(Node *node);
	DECLARE_VISIT(Switch)

  // vars
	OwnerNodeList &clone_list; //!< Aggregation, Cloner does not own the data
	std::unordered_map<Node*,Node*> old_hash; //!< Map from old to new nodes
	std::unordered_map<Node*,Node*> new_hash; //!<  "    "  new to old   " 
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
