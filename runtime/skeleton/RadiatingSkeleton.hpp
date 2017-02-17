/**
 * @file	RadiatingSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernels from skeletons
 *
 * TODO: study 2D parallel-scan, might be an option to improve performance
 *       a 2D parallel-scan would be much more affected by approximations !!
 *       The error can be corrected by combining 3 moved viewsheds (and would still be faster)
 */

#ifndef MAP_RUNTIME_SKELETON_RADIAL_HPP_
#define MAP_RUNTIME_SKELETON_RADIAL_HPP_

#include "Skeleton.hpp"
#include "../Direction.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct RadiatingSkeleton : public Skeleton
{
  // constructor and main function
	RadiatingSkeleton(Version *ver);
	void generate();

  // methods
	std::string versionCode(RadialCase rcase, Direction fst, Direction snd);

  // visit
	DECLARE_VISIT(RadialScan)

  // vars
	std::vector<RadialScan*> radia; //!< Stores RadialScan nodes
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
