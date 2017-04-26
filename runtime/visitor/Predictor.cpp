/**
 * @file    Predictor.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: could the statistics (min,max,etc) be propagated down the group too ?
 */

#include "Predictor.hpp"
#include "../Runtime.hpp"
#include <algorithm>


namespace map { namespace detail {

Predictor::Predictor(Group *group)
	: group(group)
{ }

void Predictor::clear() {
	hash.clear();
}

void Predictor::predict(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	if (not Runtime::getConfig().prediction)
		return;

	clear();

	// Fills inputs first with 'in_blk'
	for (auto in : in_blk) {
		if (in->holdtype() == HOLD_0) // When the block is null, looks for the central block
		{
			auto pred = [&](const Block *b){ return b->key == Key{in->key.node,coord}; };
			auto it = std::find_if(in_blk.begin(),in_blk.end(),pred);
			assert(it != in_blk.end());

			hash[in->key] = {(*it)->value,(*it)->fixed};
		}
		else // HOLD_1 or HOLD_N
		{
			hash[in->key] = {in->value,in->fixed};
		}
	}

	// Iterates the nodes to fill 'value_list' and 'fixed_list'
	NodeList nodes_to_fill = full_unique_join(group->nodeList(),group->outputList());

	for (auto node : nodes_to_fill) {
		node->computeFixed(coord,hash);
	}

	// Transfer outputs to 'out_blk'
	for (auto out : out_blk) {
		assert(hash.find(out->key) != hash.end());
		auto vf = hash[out->key];

		if (vf.fixed) {
			out->fixValue(vf.value);
			out->stats.active = true;
			out->stats.max = vf.value.get();
			out->stats.min = vf.value.get();
			out->stats.mean = vf.value.get();
			out->stats.std = vf.value.get();
		}
	}
}

} } // namespace map::detail
