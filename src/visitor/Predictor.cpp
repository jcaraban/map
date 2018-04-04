/**
 * @file    Predictor.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: consider moving the runtime prediction logic outside Task::preLoad
 */

#include "Predictor.hpp"
#include "../Runtime.hpp"
#include <algorithm>


namespace map { namespace detail {

Predictor::Predictor(Cluster *cluster)
	: cluster(cluster)
{ }

void Predictor::clear() {
	hash.clear();
}

void Predictor::predict(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	
	// fill with Task::preLoad, in case such logic is moved out of Task
}

} } // namespace map::detail
