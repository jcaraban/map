/**
 * @file	Exporter.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Exporter.hpp"
#include <iostream>
#include <fstream>


namespace map { namespace detail {

Exporter::Exporter(std::unordered_map<Node*,GroupList> &group_list_of)
	: group_list_of(group_list_of)
{ }

void Exporter::clear() {
	visited.clear();
	order.clear();
}

void Exporter::exportDag(NodeList node_list) {
	clear();
	
	depth = 0;
	maxd = 0;
	order.clear();

	 // Visits node by node, storing their info
	for (auto node : node_list)
		static_visit(node);

	// Reverse pass to move down nodes and shorten their edges
	for (auto it=node_list.rbegin(); it!=node_list.rend(); it++) {
		Node *node = *it;
		depth = maxd;
		for (auto next : node->nextList()) {
			auto it = depthMap.find(next->id);
			assert(it != depthMap.end());
			depth = std::min(depth, it->second-1);
		}
		depthMap[node->id] = depth;
	}

	// Gives horizontal order to the nodes
	order.resize(maxd+1,0);
	for (auto &row : vecNodes) {
		int idx = depthMap[row.id];
		row.pos = ++order[idx];
	}

	writeDag();
}

void Exporter::static_visit(Node *node) {
	depth = 0;
	for (auto prev : node->prevList()) {
		depth = std::max(depth, depthMap[prev->id]+1);
	}
	maxd = std::max(maxd,depth);
	depthMap[node->id] = depth;

	// Gets group
	Group *group = (group_list_of[node].empty()) ? nullptr : group_list_of[node].front();
	int group_id = (group == nullptr) ? -1 : group->id;

	// Adds node
	vecNodes.push_back( RowN{node->id, node->shortName(), group_id, 0} );

	// Adds edges
	for (auto &next : node->nextList()) {
		float weight = node->pattern()==FREE && !node->isInput() ? 0.1 : 1.0;
		vecEdges.push_back( RowE{node->id, next->id, weight} );
	}
}

void Exporter::writeDag() {
	std::ofstream fileNodes("DagN.csv");
	std::ofstream fileEdges("DagE.csv");

	fileNodes << "Id,Label,Group,Order" << std::endl << std::endl;
	fileEdges << "Source,Target,Weight" << std::endl << std::endl;

	for (auto &row : vecNodes)
		fileNodes << row.id << ", " << row.name << " ," << row.group << "," << row.pos << std::endl;

	for (auto &row : vecEdges)
		fileEdges << row.source << "," << row.target << "," << row.weight << std::endl;
	
	fileNodes << std::endl;
	for (int i=0; i<maxd; i++)
		fileNodes << "d"+std::to_string(i) << ",,,0" << std::endl;

	fileEdges << std::endl;
	for (int i=0; i<maxd; i++)
		fileEdges << "d"+std::to_string(i) << "," << "d"+std::to_string(i+1) << "," << "0.01" << std::endl;

	fileEdges << std::endl;
	for (auto &pair : depthMap)
		fileEdges << "d"+std::to_string(pair.second) << "," << pair.first << "," << "0.01" << std::endl;

	fileNodes.close();
	fileEdges.close();
}

#define DEFINE_VISIT(class) \
	void Exporter::visit(class *node) { \
		helper<class>(node); \
	}

	//DEFINE_VISIT(...)
#undef DEFINE_VISIT

} } // namespace map::detail
