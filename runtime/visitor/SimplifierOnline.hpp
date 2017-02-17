/**
 * @file	SimplifierOnline.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the DAG that erases redundant nodes from the main list of nodes
 *
 * Note: meant to be used 'online', this is, on the fly while adding nodes. This imposes restrictions:
 *   - The callee node has the latest/greatest id number
 *   - The callee has no next nodes hanging from it
 *
 * Based on "Global value numbering", Cliff Click, PLDI '95
 *
 * TODO: simplify the code by using a single more dynamic hash that cover all nodes
 *       this will require rethinking Node:Key, as the hash depends on the specific node
 *       special attention too with associative operations, like ADD and MUL
 */

#ifndef MAP_RUNTIME_VISITOR_SIMPLIFIER_ONLINE_HPP_
#define MAP_RUNTIME_VISITOR_SIMPLIFIER_ONLINE_HPP_

#include "Visitor.hpp"
#include <unordered_map>
#include <functional>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);
#define DECLARE_MAP(class) std::unordered_map<class::Key,class*,class::Hash> class##Map;

/*
 * Simplifies the DAG by eliminating duplicated nodes
 */
struct SimplifierOnline : public Visitor
{
  // constructor and main function
	SimplifierOnline(OwnerNodeList &node_list);
	Node* simplify(Node *node);
	void drop(Node *node);

  // methods
	void clear();

  // helper
	template <typename T> void helper(T *node, std::unordered_map<typename T::Key,T*,typename T::Hash> &map);
	template <typename T> void drop_helper(T *node, std::unordered_map<typename T::Key,T*,typename T::Hash> &map);

  // declarations
	DECLARE_MAP(Constant)
	DECLARE_MAP(Rand)
	DECLARE_MAP(Index)
	DECLARE_MAP(Cast)
	DECLARE_MAP(Unary)
	DECLARE_MAP(Binary)
	DECLARE_MAP(Conditional)
	DECLARE_MAP(Diversity)
	DECLARE_MAP(Neighbor)
	DECLARE_MAP(BoundedNbh)
	DECLARE_MAP(SpreadNeighbor)
	DECLARE_MAP(Convolution)
	DECLARE_MAP(FocalFunc)
	DECLARE_MAP(FocalPercent)
	DECLARE_MAP(FocalFlow)
	DECLARE_MAP(ZonalReduc)
	DECLARE_MAP(RadialScan)
	DECLARE_MAP(SpreadScan)
	DECLARE_MAP(Loop)
	DECLARE_MAP(Access)
	DECLARE_MAP(LhsAccess)
	DECLARE_MAP(Read)
	DECLARE_MAP(Write)
	DECLARE_MAP(Scalar)
	DECLARE_MAP(Checkpoint)
	DECLARE_MAP(Stats)
	DECLARE_MAP(Barrier)

  // visit
	//void static_visit(Node *node);
	DECLARE_VISIT(Constant)
	DECLARE_VISIT(Rand)
	DECLARE_VISIT(Index)
	DECLARE_VISIT(Cast)
	DECLARE_VISIT(Unary)
	DECLARE_VISIT(Binary)
	DECLARE_VISIT(Conditional)
	DECLARE_VISIT(Diversity)
	DECLARE_VISIT(Neighbor)
	DECLARE_VISIT(BoundedNbh)
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
	
	DECLARE_VISIT(Temporal)

  // vars
	OwnerNodeList &node_list; //!< Aggregation, Simplifier does not own the data
	Node *orig;
	bool dropping;
};

#undef DECLARE_VISIT
#undef DECLARE_MAP

} } // namespace map::detail

#endif

#include "SimplifierOnline.tpl"
