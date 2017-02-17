/**
 * @file	Partitioner.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Breaks down the list of nodes in smaller ones so that some limits and constrains are not surpassed
 */

#ifndef MAP_RUNTIME_VISITOR_PARTITIONER_HPP_
#define MAP_RUNTIME_VISITOR_PARTITIONER_HPP_

#include "Visitor.hpp"
#include <unordered_map>
#include <unordered_set>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Return an ordered list (top-down) of the visited nodes
 */
struct Partitioner
{
  // constructor and main function
	Partitioner();
	std::vector<NodeList> split(NodeList list);

  // methods
	void clear();

  // vars
	const int hard, soft;
	std::unordered_map<Node*,std::unordered_set<Node*>> dep_hash; //!< Hash of open dependencies
	struct CutStruct {
		int idx; //!< Index marking the node after which to cut
		int cost; //!< Cost of the cut in terms of open dependencies
		NodeList dep; //!< Vector of open dependencies
	};
	std::vector<CutStruct> cut_list;
	int min_idx, min_cost;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
