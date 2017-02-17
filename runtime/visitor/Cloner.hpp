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

  // helper
	template <typename T> void helper(T *node);

  // visit
	DECLARE_VISIT(Constant)
	DECLARE_VISIT(Rand)
	DECLARE_VISIT(Index)
	DECLARE_VISIT(Cast)
	DECLARE_VISIT(Unary)
	DECLARE_VISIT(Binary)
	DECLARE_VISIT(Conditional)
	DECLARE_VISIT(Diversity)
	DECLARE_VISIT(Neighbor)
	DECLARE_VISIT(SpreadNeighbor)
	DECLARE_VISIT(Convolution)
	DECLARE_VISIT(FocalFunc)
	DECLARE_VISIT(FocalPercent)
	DECLARE_VISIT(FocalFlow)
	DECLARE_VISIT(ZonalReduc)
	DECLARE_VISIT(RadialScan)
	DECLARE_VISIT(SpreadScan)
	DECLARE_VISIT(Loop)
	DECLARE_VISIT(Access)
	DECLARE_VISIT(LhsAccess)
	DECLARE_VISIT(Read)
	DECLARE_VISIT(Write)
	DECLARE_VISIT(Scalar)
	DECLARE_VISIT(Checkpoint)
	DECLARE_VISIT(Stats)
	DECLARE_VISIT(Barrier)

  // vars
	OwnerNodeList &new_list; //!< Aggregation, Cloner does not own the data
	std::unordered_map<Node*,Node*> old_hash; //!< Map from old to new nodes
	std::unordered_map<Node*,Node*> new_hash; //!<  "    "  new to old   " 
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
