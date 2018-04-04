/**
 * @file	Skeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_SECTION_LOOP_HPP_
#define MAP_RUNTIME_SECTION_LOOP_HPP_

#include "../Section.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 *
 */
struct LoopSection : public Section
{
  // codegen
	std::string section() override;

  // visit
	DECLARE_VISIT(LoopCond)

  // vars
	//std::vector<SkelReduc> reduc_list; //!< Stores reductions nodes
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
