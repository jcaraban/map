/**
 * @file	RadialSkeleton.hpp 
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

#include "../Skeleton.hpp"
#include "../../util/Direction.hpp"


namespace map { namespace detail {

struct RadialSkeleton : public Skeleton
{
  // constructor and main function
	RadialSkeleton(Version *ver);
	std::string generate();

  // methods
	std::string generateCode(RadialCase rcase, Direction fst, Direction snd);

  // vars
	//std::vector<RadialScan*> radia; //!< Stores RadialScan nodes
};

} } // namespace map::detail

#endif
