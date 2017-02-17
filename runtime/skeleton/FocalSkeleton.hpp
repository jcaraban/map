/**
 * @file	FocalSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernel codes from skeletons
 *
 * TODO: the halos should be computed first, then the center. This way the scalar variables
 *       keep the right values and they can be reused in the POSCORE section
 *       Now inputs are read in POSCORE just in case, even when not needed
 * TODO: try filtering the access to halos by Group position first (sort of like CpuFocal)
 */

#ifndef MAP_RUNTIME_SKELETON_FOCAL_HPP_
#define MAP_RUNTIME_SKELETON_FOCAL_HPP_

#include "Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct FocalSkeleton : public Skeleton
{
  // constructor and main function
	FocalSkeleton(Version *ver);
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
	
  // vars
	std::vector<std::pair<Mask,int>> mask; //!< Stores pairs {mask,id}
	std::vector<Neighbor*> nbh; //!< Stores Neighbor nodes
	std::vector<Convolution*> conv; //!< Stores Convolution nodes
	std::vector<FocalFunc*> func; //!< Stores FocalFunc nodes
	std::vector<FocalPercent*> percent; //!< Stores FocalPercent nodes
	std::vector<FocalFlow*> flow; //!< Stores FocalFlow nodes
	int level;
	std::vector<BlockSize> halo; //< Stores the halo of each level
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
