/**
 * @file	ListerBU.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the DAG in ascending bottom-up order that return an ordered list of the visited nodes
 */

#ifndef MAP_RUNTIME_VISITOR_LISTERBU_HPP_
#define MAP_RUNTIME_VISITOR_LISTERBU_HPP_

#include "Visitor.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Return an ordered list (top-down) of the visited nodes
 */
struct ListerBU : public Visitor
{
  // constructor and main function
	ListerBU();
	NodeList list(Node *node);
	NodeList list(NodeList few_nodes);

  // methods
	void clear();

  // helper
	template <typename T> void helper(T *node);

  // visit
	void static_visit(Node *node);
	//DECLARE_VISIT(...)

  // vars
	NodeList node_list;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_RUNTIME_VISITOR_SIMPLIFIERBU_TPL_
#define MAP_RUNTIME_VISITOR_SIMPLIFIERBU_TPL_

namespace map { namespace detail {

template <typename T>
void ListerBU::helper(T *node) {
	if (wasVisited(node)) return;
	setVisited(node);
	
	node->goUp(this);
	node_list.push_back(node);
}

} } // namespace map::detail

#endif
