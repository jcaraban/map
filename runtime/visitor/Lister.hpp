/**
 * @file	Lister.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the graph in backward order that return an list of nodes involved in the computation
 * Note: all non-visited nodes are ignored, as if "dead nodes elimination" was applied
 */

#ifndef MAP_RUNTIME_VISITOR_LISTER_HPP_
#define MAP_RUNTIME_VISITOR_LISTER_HPP_

#include "Visitor.hpp"
#include <queue>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Return a list only with the visited nodes
 */
struct Lister : public Visitor
{
  // constructor and main function
	Lister();
	NodeList list(Node *node);
	NodeList list(NodeList few_nodes);

  // methods
	void clear();

  // vars
	NodeList node_list;
	std::queue<Node*> queue; //!< Queue of 'prev' / ' forw' nodes still to be visited
	std::unordered_set<Node*> marked; //!< Mark nodes in the middle of a visit, for loop detection
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
