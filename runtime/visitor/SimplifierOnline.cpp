/**
 * @file	SimplifierOnline.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "SimplifierOnline.hpp"
#include "../Runtime.hpp"
#include <iostream>


namespace map { namespace detail {

SimplifierOnline::SimplifierOnline(OwnerNodeList &node_list)
	: node_list(node_list)
{
	clear();
}

void SimplifierOnline::clear() {
	orig = nullptr;
	ConstantMap.clear();
	RandMap.clear();
	IndexMap.clear();
	CastMap.clear();
	UnaryMap.clear();
	BinaryMap.clear();
	ConditionalMap.clear();
	DiversityMap.clear();
	NeighborMap.clear();
	BoundedNbhMap.clear();
	SpreadNeighborMap.clear();
	ConvolutionMap.clear();
	FocalFuncMap.clear();
	FocalPercentMap.clear();
	FocalFlowMap.clear();
	ZonalReducMap.clear();
	RadialScanMap.clear();
	SpreadScanMap.clear();
	LoopMap.clear();
	LoopCondMap.clear();
	LoopHeadMap.clear();
	LoopTailMap.clear();
	FeedbackMap.clear();
	AccessMap.clear();
	LhsAccessMap.clear();
	ReadMap.clear();
	WriteMap.clear();
	ScalarMap.clear();
	CheckpointMap.clear();
	StatsMap.clear();
	BarrierMap.clear();
}

Node* SimplifierOnline::simplify(Node *node) {
	dropping = false;
	node->accept(this); // Simplifies node, stores original in 'orig'
	return orig;
}

void SimplifierOnline::drop(Node *node) {
	dropping = true;
	node->accept(this); // Drops node
}

#define DEFINE_VISIT(class) \
	void SimplifierOnline::visit(class *node) { \
		if (dropping) \
			drop_helper<class>(node,class##Map); \
		else \
			helper<class>(node,class##Map); \
	}
	
	DEFINE_VISIT(Constant)
	DEFINE_VISIT(Rand)
	DEFINE_VISIT(Index)
	DEFINE_VISIT(Cast)
	DEFINE_VISIT(Unary)
	DEFINE_VISIT(Binary)
	DEFINE_VISIT(Conditional)
	DEFINE_VISIT(Diversity)
	DEFINE_VISIT(Neighbor)
	DEFINE_VISIT(BoundedNbh)
	DEFINE_VISIT(SpreadNeighbor)
	DEFINE_VISIT(Convolution)
	DEFINE_VISIT(FocalFunc)
	DEFINE_VISIT(FocalPercent)
	DEFINE_VISIT(FocalFlow)
	DEFINE_VISIT(ZonalReduc)
	DEFINE_VISIT(RadialScan)
	DEFINE_VISIT(SpreadScan)
	DEFINE_VISIT(Loop)
	DEFINE_VISIT(LoopCond)
	DEFINE_VISIT(LoopHead)
	DEFINE_VISIT(LoopTail)
	DEFINE_VISIT(Feedback)
	DEFINE_VISIT(Access)
	DEFINE_VISIT(LhsAccess)
	DEFINE_VISIT(Read)
	DEFINE_VISIT(Write)
	DEFINE_VISIT(Scalar)
	DEFINE_VISIT(Checkpoint)
	DEFINE_VISIT(Stats)
	DEFINE_VISIT(Barrier)
#undef DEFINE_VISIT

void SimplifierOnline::visit(Temporal *node) { orig = node; }

} } // namespace map::detail
