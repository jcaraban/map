/**
 * @file	LoopSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_SKELETON_LOOP_HPP_
#define MAP_RUNTIME_SKELETON_LOOP_HPP_

#include "Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * 
 */
struct LoopSkeleton : public Skeleton
{
	LoopSkeleton(Version *ver);
	std::string generate();

  // methods
	std::string versionCode();

  // visit
	void visit_input(Node *node);
	void visit_output(Node *node);
	DECLARE_VISIT(LoopCond)
	//DECLARE_VISIT(LoopHead)
	//DECLARE_VISIT(LoopTail)
	DECLARE_VISIT(Merge)
	DECLARE_VISIT(Switch)

  // vars
	//std::vector<Merge*> merge_list;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
