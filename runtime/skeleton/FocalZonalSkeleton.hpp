/**
 * @file	FocalZonalSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernel codes from skeletons
 */

#ifndef MAP_RUNTIME_SKELETON_FOCAL_ZONAL_HPP_
#define MAP_RUNTIME_SKELETON_FOCAL_ZONAL_HPP_

#include "Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct FocalZonalSkeleton : public Skeleton
{
  // constructor and main function
	FocalZonalSkeleton(Version *ver);
	void generate();

  // methods
	void compact();
	std::string versionCode();

  // visit
	DECLARE_VISIT(Neighbor)
	DECLARE_VISIT(Convolution)
	DECLARE_VISIT(FocalFunc)
	DECLARE_VISIT(FocalPercent)
	DECLARE_VISIT(FocalFlow)

	DECLARE_VISIT(ZonalReduc)
	
  // vars
	std::vector<std::pair<Mask,int>> mask; //!< Stores pairs {mask,id}
	std::vector<Neighbor*> nbh; //!< Stores Neighbor nodes
	std::vector<Convolution*> conv; //!< Stores Convolution nodes
	std::vector<FocalFunc*> func; //!< Stores FocalFunc nodes
	std::vector<FocalPercent*> percent; //!< Stores FocalPercent nodes
	std::vector<FocalFlow*> flow; //!< Stores FocalFlow nodes
	int level;
	std::vector<BlockSize> halo; //< Stores the halo of each level

	std::vector<ZonalReduc*> reduc; //!< Stores ZonalReduc nodes
	std::string zonal_code;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
