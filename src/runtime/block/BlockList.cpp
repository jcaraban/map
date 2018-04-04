/**
 * @file	BlockList.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "BlockList.hpp"


namespace map { namespace detail {

void BlockList::preloadInputs() {
	for (auto &iblk : list)
		iblk->preload();
}

void BlockList::evictEntries() {
	for (auto &iblk : list)
		iblk->evict();
}

void BlockList::loadInputs() {
	for (auto &iblk : list)
		iblk->load();
}

void BlockList::initOutputs() {
	for (auto &oblk : list)
		oblk->init();
}

void BlockList::storeOutputs() {
	for (auto &oblk : list)
		oblk->store();
}

void BlockList::reduceOutputs() {
	for (auto &oblk : list)
		oblk->reduce();
}

} } // namespace map::detail
