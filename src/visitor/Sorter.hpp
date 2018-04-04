/**
 * @file	Sorter.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Forward visitor that reorders the list by {dependencies,id}
 */

#ifndef MAP_RUNTIME_VISITOR_SORTER_HPP_
#define MAP_RUNTIME_VISITOR_SORTER_HPP_

#include "Visitor.hpp"
#include <unordered_map>
#include <unordered_set>
#include <queue>


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

  // vars
	NodeList node_list;
	std::priority_queue<Node*,std::vector<Node*>,node_id_greater> prique;
	std::unordered_map<Node*,int> prev_count; //!< Keeps the count of remaining prevs
	std::unordered_set<Node*> included;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
