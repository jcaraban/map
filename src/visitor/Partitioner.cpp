/**
 * @file	Partitioner.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Partitioner.hpp"
#include "../Runtime.hpp"
#include <limits>
#include <algorithm>


namespace map { namespace detail {

Partitioner::Partitioner()
	: hard(Runtime::getConfig().hard_nodes_limit)
	, soft(Runtime::getConfig().soft_nodes_limit)
{ }

void Partitioner::clear() {
	//visited.clear();
	dep_hash.clear();
	cut_list.clear();
	min_idx = 0;
	min_cost = std::numeric_limits<int>::max();
}

std::vector<NodeList> Partitioner::split(NodeList list) {
	std::vector<NodeList> multi_list;
	//auto isFreeOrScalar = [](Node *node) { return node->pattern()==FREE || node->numdim()==D0; };

	while (!list.empty())
	{
		if (list.size() < hard) // No partitioning needed
		{
			multi_list.push_back(list);
			list.clear();
		}
		else
		{	
			clear(); // clear structures
			int non_free_count = 0;

			for (int i=0; i<hard; i++) {
				auto node = list[i];
				// Removes dependencies of prev nodes
				for (auto prev : node->prevList()) {
					dep_hash[prev].erase(node);
					if (dep_hash[prev].empty()) {
						dep_hash.erase(prev);
						if (prev->pattern() != FREE)
							non_free_count--;
					}
				}
				// Marks all next depdencies
				for (auto next : node->nextList()) {
					dep_hash[node].insert(next);
				}
				// Counts the non-free open dependencies 
				if (node->pattern() != FREE) {
					non_free_count++; 
				}
				// Stores the cuts of minimal cost
				if (i > soft) {
					int cost = non_free_count;
					if (cost < min_cost) {
						min_idx = i;
						min_cost = cost;
						cut_list.clear();
					}
					if (cost <= min_cost) {
						cut_list.push_back({i,cost,NodeList()});
						for (auto &entry : dep_hash)
							cut_list.back().dep.push_back(entry.first);
					}
				}
			}

			assert(cut_list.back().cost == 1);
			// How to pick a good cut?
			auto cut = cut_list.back(); // @ Picking the last now
			Node *checkpoint  = Checkpoint::Factory(list[cut.idx]);
			dynamic_cast<Checkpoint*>(checkpoint)->setFilled();

			NodeList left, right;
			left.insert(left.end(),list.begin(),list.begin()+cut.idx+1);
			left.push_back(checkpoint);

			right.insert(right.end(),cut.dep.begin(),cut.dep.end());
			remove_value(list[cut.idx],right); // Only the free nodes
			right.push_back(checkpoint);
			right.insert(right.end(),list.begin()+cut.idx+1,list.end());
			
			multi_list.push_back(left);
			list = right;
		}
	}

	return multi_list;
}

} } // namespace map::detail
