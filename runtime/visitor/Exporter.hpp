/**
 * @file	Exporter.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the DAG that export the nodes to a CSV file
 */

#ifndef MAP_RUNTIME_VISITOR_EXPORTER_HPP_
#define MAP_RUNTIME_VISITOR_EXPORTER_HPP_

#include "Visitor.hpp"
#include <vector>
#include <map>
#include <unordered_map>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

struct RowN {
	int id; //!< Unique idenfitifer
	std::string name; //!< Node name
	int group; //!< Group id
	int pos; //!< Row position
};

struct RowE {
	int source; //!< From source
	int target; //!> To target
	float weight; //!> Edge weight
};

/*
 * Exports the graph to a csv file
 * Graph visualization software can be used to display the cvs
 * e.g. Gephi 0.82
 */
struct Exporter : public Visitor
{
  // constructor and main function
	Exporter(std::unordered_map<Node*,GroupList> &group_list_of);
	void exportDag(NodeList node_list);

  // methods
	void clear();
	void writeDag();

  // helper
	template <typename T> void helper(T *node);

  // visit
	void static_visit(Node *node);
	//DECLARE_VISIT(...)

  // vars
	std::unordered_map<Node*,GroupList> &group_list_of; // aggregation

	std::vector<RowN> vecNodes;
	std::vector<RowE> vecEdges;
	int depth, maxd;
	std::vector<int> order;
	std::map<int,int> depthMap;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif

//-------------------------------------------------------------------------------------------------------------------//

#ifndef MAP_RUNTIME_VISITOR_EXPORTERBU_TPL_
#define MAP_RUNTIME_VISITOR_EXPORTERBU_TPL_

namespace map { namespace detail {

template <typename T>
void Exporter::helper(T *node) {
	if (wasVisited(node))
		assert(0);

	depth = 0;
	for (auto prev : node->prevList()) {
		depth = std::max(depth, depthMap[prev->id]+1);
	}
	maxd = std::max(maxd,depth);
	depthMap[node->id] = depth;

	// Gets group
	Group *group = (group_list_of[node].empty()) ? nullptr : group_list_of[node].front();

	// Adds node
	vecNodes.push_back( RowN{node->id, node->shortName(), group->id, 0} );

	// Adds edges
	for (auto &next : node->nextList()) {
		float weight = node->numdim()==D0 ? 0.1 : 1.0;
		vecEdges.push_back( RowE{node->id, next->id, weight} );
	}

	setVisited(node);
}

} } // namespace map::detail

#endif
