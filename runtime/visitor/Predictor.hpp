/**
 * @file    Predictor.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_VISITOR_PREDICTOR_HPP_
#define MAP_RUNTIME_VISOTOR_PREDICTOR_HPP_

#include "Visitor.hpp"
#include <unordered_map>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 *
 */
struct Predictor : public Visitor
{
  // constructor and main function
	Predictor(Group *group);
	bool predict(Coord coord, const BlockList &in_blk, const BlockList &out_blk);

  // methods
	void clear();

  // visit
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
	DECLARE_VISIT(SpreadNeighbor)
	DECLARE_VISIT(Convolution)
	DECLARE_VISIT(FocalFunc)
	DECLARE_VISIT(FocalPercent)
	DECLARE_VISIT(FocalFlow)
	DECLARE_VISIT(ZonalReduc)
	DECLARE_VISIT(RadialScan)
	DECLARE_VISIT(SpreadScan)
	DECLARE_VISIT(Access)
	DECLARE_VISIT(LhsAccess)
	DECLARE_VISIT(Read)
	DECLARE_VISIT(Write)
	DECLARE_VISIT(Scalar)
	DECLARE_VISIT(Temporal)
	DECLARE_VISIT(Stats)
	DECLARE_VISIT(Barrier)
	DECLARE_VISIT(Checkpoint)

  // vars
	Group *group;
	Coord coord;

	struct ValFix { VariantType value; bool fixed; };
	std::unordered_map<Key,ValFix,key_hash> hash;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
