/**
 * @file	Temporal.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: using Temporal nodes goes against the model, operations requiring one should be decomposed
 */

#ifndef MAP_RUNTIME_DAG_TEMPORAL_HPP_
#define MAP_RUNTIME_DAG_TEMPORAL_HPP_

#include "IO.hpp"


namespace map { namespace detail {

struct Temporal : public Node
{
	// Internal declarations
	
	// Constructors & methods
	Temporal(const MetaData &meta);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;

	bool isTemporal() const { return true; }
};

} } // namespace map::detail

#endif
