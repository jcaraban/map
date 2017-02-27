/**
 * @file	Lister.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the DAG in ascending bottom-up order that return an ordered list of the visited nodes
 * Note: consequently all non-visited nodes are ignore, as if "dead nodes elimination" was applied
 */

#ifndef MAP_RUNTIME_VISITOR_LISTER_HPP_
#define MAP_RUNTIME_VISITOR_LISTER_HPP_

#include "Visitor.hpp"
#include <unordered_set>
#include <unordered_map>


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
	bool wasMarked(Node *node);
	void setMarked(Node *node);
	void unMarked(Node *node);

  // helper
	template <typename T> void helper(T *node);

  // visit
	void static_visit(Node *node);
	//DECLARE_VISIT(...)

  // vars
	NodeList node_list;
	std::unordered_set<Node*> marked; //!< Mark nodes in the middle of a visit, for loop detection
	std::unordered_map<Node*,NodeList> isonext; //!< List of isolated 'nexts' due to this node
	std::unordered_map<Node*,NodeList> isoprev; //!< List of 'prevs' holding this node isolated
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_RUNTIME_VISITOR_SIMPLIFIERBU_TPL_
#define MAP_RUNTIME_VISITOR_SIMPLIFIERBU_TPL_

namespace map { namespace detail {

template <typename T>
void Lister::helper(T *node) {
	if (wasVisited(node)) return;
	setVisited(node);
	
	node->goUp(this);
	node_list.push_back(node);
}

} } // namespace map::detail

#endif
