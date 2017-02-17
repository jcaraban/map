/**
 * @file	SpreadingSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernel code from skeletons
 */

#ifndef MAP_RUNTIME_SKELETON_SPREAD_HPP_
#define MAP_RUNTIME_SKELETON_SPREAD_HPP_

#include "Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct SpreadingSkeleton : public Skeleton
{
  // constructor and main function
	SpreadingSkeleton(Version *ver);
	void generate();

  // methods
	std::string versionCode();

  // visit
	DECLARE_VISIT(SpreadScan)

  // vars
	std::vector<SpreadScan*> spread;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
