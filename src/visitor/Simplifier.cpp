/**
 * @file	Simplifier.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Simplifier.hpp"
#include <iostream>


namespace map { namespace detail {

Simplifier::Simplifier(OwnerNodeList &node_list)
	: node_list(node_list)
{
	clear();
}

void Simplifier::clear() {
	orig = nullptr;
	ConstantMap.clear();
	IndexMap.clear();
	RandMap.clear();
	CastMap.clear();
	UnaryMap.clear();
	BinaryMap.clear();
	ConditionalMap.clear();
	DiversityMap.clear();
	NeighborMap.clear();
	BoundedNeighborMap.clear();
	SpreadNeighborMap.clear();
	ConvolutionMap.clear();
	FocalFuncMap.clear();
	FocalPercentMap.clear();
	ZonalReducMap.clear();
	RadialScanMap.clear();
	SpreadScanMap.clear();
	LoopCondMap.clear();
	LoopHeadMap.clear();
	LoopTailMap.clear();
	MergeMap.clear();
	SwitchMap.clear();
	AccessMap.clear();
	LhsAccessMap.clear();
	ReadMap.clear();
	WriteMap.clear();
	ScalarMap.clear();
	CheckpointMap.clear();
	BarrierMap.clear();
	SummaryMap.clear();
	DataSummaryMap.clear();
	BlockSummaryMap.clear();
	GroupSummaryMap.clear();
}

Node* Simplifier::simplify(Node *node) {
	dropping = false;
	node->accept(this); // Simplifies node, stores original in 'orig'
	return orig;
}

void Simplifier::drop(Node *node) {
	dropping = true;
	node->accept(this); // Drops node
}

#define DEFINE_VISIT(class) \
	void Simplifier::visit(class *node) { \
		if (dropping) \
			drop_helper<class>(node,class##Map); \
		else \
			helper<class>(node,class##Map); \
	}
	
	DEFINE_VISIT(Constant)
	DEFINE_VISIT(Empty)
	DEFINE_VISIT(Index)
	DEFINE_VISIT(Identity)
	DEFINE_VISIT(Rand)
	DEFINE_VISIT(Cast)
	DEFINE_VISIT(Unary)
	DEFINE_VISIT(Binary)
	DEFINE_VISIT(Conditional)
	DEFINE_VISIT(Diversity)
	DEFINE_VISIT(Neighbor)
	DEFINE_VISIT(BoundedNeighbor)
	DEFINE_VISIT(SpreadNeighbor)
	DEFINE_VISIT(Convolution)
	DEFINE_VISIT(FocalFunc)
	DEFINE_VISIT(FocalPercent)
	DEFINE_VISIT(ZonalReduc)
	DEFINE_VISIT(RadialScan)
	DEFINE_VISIT(SpreadScan)
	DEFINE_VISIT(LoopCond)
	DEFINE_VISIT(LoopHead)
	DEFINE_VISIT(LoopTail)
	DEFINE_VISIT(Merge)
	DEFINE_VISIT(Switch)
	DEFINE_VISIT(Access)
	DEFINE_VISIT(LhsAccess)
	DEFINE_VISIT(Read)
	DEFINE_VISIT(Write)
	DEFINE_VISIT(Scalar)
	DEFINE_VISIT(Checkpoint)
	DEFINE_VISIT(Barrier)
	DEFINE_VISIT(Summary)
	DEFINE_VISIT(DataSummary)
	DEFINE_VISIT(BlockSummary)
	DEFINE_VISIT(GroupSummary)
#undef DEFINE_VISIT

void Simplifier::visit(Temporal *node) { orig = node; }

} } // namespace map::detail
