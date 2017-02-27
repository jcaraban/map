/**
 * @file	Sorter.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the DAG that reorders the input list by {dependencies,id}
 */

#ifndef MAP_RUNTIME_VISITOR_SORTER_HPP_
#define MAP_RUNTIME_VISITOR_SORTER_HPP_

#include "Visitor.hpp"
#include <unordered_map>
#include <unordered_set>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Return a sorted list. Sorts by dependency 1st, by id 2nd
 */
struct Sorter : public Visitor
{
  // constructor and main function
	Sorter();
	NodeList sort(NodeList list);

  // methods
	void clear();

  // helper
	template <typename T> void helper(T *node);

  // visit
	void static_visit(Node *node);
	//DECLARE_VISIT(...)

  // vars
	NodeList node_list;
	std::unordered_map<Node*,std::unordered_set<Node*>> prev_hash; //!< LookUp for prev of prev
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_RUNTIME_VISITOR_SIMPLIFIERBU_TPL_
#define MAP_RUNTIME_VISITOR_SIMPLIFIERBU_TPL_

namespace map { namespace detail {

template <typename T>
void Sorter::helper(T *node) {
	if (wasVisited(node)) return;
	setVisited(node);
	
	node->goUp(this);
	node_list.push_back(node);
}

} } // namespace map::detail

#endif
