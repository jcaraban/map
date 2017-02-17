/**
 * @file	ZonalSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernel codes from skeletons
 */

#ifndef MAP_RUNTIME_SKELETON_ZONAL_HPP_
#define MAP_RUNTIME_SKELETON_ZONAL_HPP_

#include "Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct ZonalSkeleton : public Skeleton
{
  // constructor and main function
	ZonalSkeleton(Version *ver);
	void generate();

  // methods
	std::string versionCode();

  // visit
	DECLARE_VISIT(ZonalReduc)
	DECLARE_VISIT(Stats)
  
  // vars
	std::vector<ZonalReduc*> reduc; //!< Stores ZonalReduc nodes
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
