/**
 * @file	SpreadSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernel code from skeletons
 */

#ifndef MAP_RUNTIME_SKELETON_SPREAD_HPP_
#define MAP_RUNTIME_SKELETON_SPREAD_HPP_

#include "../Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct SpreadSkeleton : public Skeleton
{
  // constructor and main function
	SpreadSkeleton(Version *ver);
	std::string generate();

  // methods
	std::string generateCode();

  // visit
	DECLARE_VISIT(SpreadScan)

  // vars
	std::vector<SpreadScan*> spread;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
