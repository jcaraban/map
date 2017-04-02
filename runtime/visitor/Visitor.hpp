/**
 * @file	Visitor.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_RUNTIME_VISITOR_HPP
#define MAP_RUNTIME_VISITOR_HPP

#include "../dag/dag.hpp"
#include <unordered_set>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node) { assert(0); }

/*
 * Visitor
 */
struct Visitor
{
  protected:
  	std::unordered_set<Node*> visited;

  public:
  	bool wasVisited(Node* node);
  	void setVisited(Node* node);

  	void visit(Node *node) = delete;
	DECLARE_VISIT(Constant)
	DECLARE_VISIT(Index)
	DECLARE_VISIT(Identity)
	DECLARE_VISIT(Rand)
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
	DECLARE_VISIT(LoopCond)
	DECLARE_VISIT(LoopHead)
	DECLARE_VISIT(LoopTail)
	DECLARE_VISIT(Merge)
	DECLARE_VISIT(Switch)
	DECLARE_VISIT(Access)
	DECLARE_VISIT(LhsAccess)
	DECLARE_VISIT(Read)
	DECLARE_VISIT(Write)
	DECLARE_VISIT(Scalar)
	DECLARE_VISIT(Temporal)
	DECLARE_VISIT(Checkpoint)
	DECLARE_VISIT(Stats)
	DECLARE_VISIT(Barrier)
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
