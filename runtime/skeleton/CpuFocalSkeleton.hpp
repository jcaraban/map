/**
 * @file	CpuFocalSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernel codes from skeletons
 */

#ifndef MAP_RUNTIME_SKELETON_FOCAL_CPU_HPP_
#define MAP_RUNTIME_SKELETON_FOCAL_CPU_HPP_

#include "Skeleton.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct CpuFocalSkeleton : public Skeleton
{
  // constructor and main function
	CpuFocalSkeleton(Version *ver);
	std::string generate();

  // methods
	std::string versionCode();

  // visit
	DECLARE_VISIT(Neighbor)
	DECLARE_VISIT(Convolution)
	DECLARE_VISIT(FocalFunc)
	DECLARE_VISIT(FocalPercent)
	
  // vars
	std::vector<std::pair<Mask,int>> mask; //!< Stores pairs {mask,id}
	std::vector<Neighbor*> nbh; //!< Stores Neighbor nodes
	std::vector<Convolution*> conv; //!< Stores Convolution nodes
	std::vector<FocalFunc*> func; //!< Stores FocalFunc nodes
	std::vector<FocalPercent*> percent; //!< Stores FocalPercent nodes
	int level;
	//std::vector<BlockSize> halo; //!< Stores the halo of each level
	bool inner_part; //!< Signals visitors to generate code for the inner / outer part
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
