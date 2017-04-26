/**
 * @file	LocalSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernels from skeletons
 */

#ifndef MAP_RUNTIME_SKELETON_LOCAL_HPP_
#define MAP_RUNTIME_SKELETON_LOCAL_HPP_

#include "Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct LocalSkeleton : public Skeleton
{
  // constructor and main function
	LocalSkeleton(Version *ver);
	std::string generate();

  // methods
	std::string versionCode();

  // visit

  // vars
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
