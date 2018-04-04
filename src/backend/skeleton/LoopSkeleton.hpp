/**
 * @file	LoopSkeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_SKELETON_LOOP_HPP_
#define MAP_RUNTIME_SKELETON_LOOP_HPP_

#include "../Skeleton.hpp"


namespace map { namespace detail {

/*
 * 
 */
struct LoopSkeleton : public Skeleton
{
	LoopSkeleton(Version *ver);
	std::string generate();

  // methods
	std::string generateCode();

  // vars
	//std::vector<Merge*> merge_list;
};

} } // namespace map::detail

#endif
