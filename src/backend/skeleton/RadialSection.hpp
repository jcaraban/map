/**
 * @file	Skeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_SECTION_RADIAl_HPP_
#define MAP_RUNTIME_SECTION_RADIAL_HPP_

#include "../Section.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 *
 */
struct RadialSection : public Section
{
  // codegen
	std::string section() override;

  // visit
	DECLARE_VISIT(RadialScan)

  // vars
	//std::vector<RadialScan*> radia; //!< Stores RadialScan nodes
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
