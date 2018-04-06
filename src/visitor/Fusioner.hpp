/**
 * @file	Fusioner.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the DAG that fuses Nodes into Clusters
 *
 * TODO: fusioner should return 'sorted_cluster_list' and 'cluster_list_of'
 */

#ifndef MAP_RUNTIME_VISITOR_FUSIONER_HPP_
#define MAP_RUNTIME_VISITOR_FUSIONER_HPP_

#include "Visitor.hpp"
#include <unordered_set>
#include <unordered_map>
#include <functional>


namespace map { namespace detail {

typedef std::vector<std::unique_ptr<Cluster>> OwnerClusterList; // forward declaration

/*
 *
 */
struct Fusioner
{
  // constructor and main function
	Fusioner(OwnerClusterList& cluster_list);
	void fuse(NodeList list);

  // methods
	void clear();

	/*
	 * Inserts a new cluster in 'cluster_list' and returns it
	 */
	Cluster* newCluster();

	/*
	 * Deletes 'cluster' from the list of clusters
	 */
	void removeCluster(Cluster *cluster);

	/*
	 * returns true when 'top' and 'bot' can be pipe-fused following the top-bot direcction
	 */
	bool canPipeFuse(Cluster *top, Cluster *bot);

	/*
	 * returns true when 'left' and 'right' can be flat-fused
	 */
	bool canFlatFuse(Cluster *left, Cluster *right);

	/*
	 * The content of 'bot' is transferred to 'top' and it's erased
	 * Links of all involved 'prev' and 'next' nodes are updated
	 * Only valid for clusters with producer-consumer data-flow relations
	 */
	Cluster* pipeFuseCluster(Cluster *&top, Cluster *&bot);	

	/*
	 * The content of 'right' is transferred to 'left' and it's erased
	 * Links of all involved 'prev' and 'next' nodes are updated
	 * Only valid for clusters with consumer-consumer data-flow relations
	 */
	Cluster* flatFuseCluster(Cluster *&left, Cluster *&right);

	/*
	 * The content of 'other' is transferred to 'one' and it's erased
	 * Valid for any type of data-flow relations and algorithmic pattern
	 * NB: use carefully, fusing incompatible clusters will result in errors
	 */
	Cluster* freeFuseCluster(Cluster *&one, Cluster *&other);

	/*
	 *
	 */
	void process(Node *node);

	/*
	 *
	 */
	void pipeGently(Node *node);

	/*
	 *
	 */
	void flatGently(Node *node);

	/*
	 *
	 */
	void processLoop(Node *node);

	/*
	 *
	 */
	void processScalar(Cluster *cluster);

	/*
	 *
	 */
	void process(Cluster *cluster);
	void processBU(Cluster *cluster); // @

	/*
	 * For every block with only input/free-nodes, their content is forwarded
	 * to those next-blocks depending on them and they are removed
	 */
	void forwarding(std::function<bool(Node*)> pred);

	/*
	 * Marks as Input/Output all those nodes in the cluster boundary that link to other clusters
	 */
	void linking();

	/*
	 * Sorts cluster in order of dependency. Sorts nodes within clusters in order of id
	 */
	void sorting();

	/*
	 *
	 */
	ClusterList toposort(ClusterList list);

	/*
	 *
	 */
	void print(OwnerClusterList &list); // @
	void print(ClusterList &list); // @

  // vars
	OwnerClusterList &cluster_list; //!< Aggregation, Fusioner does not own the data
	std::unordered_map<Node*,ClusterList> cluster_list_of; //!< For each node, stores its clusters
	
	std::unordered_set<Cluster*> visited; // Remembers what clusters have been previously visited
	ClusterList sorted_cluster_list; //!< Stores the clusters in topological order
};

} } // namespace map::detail

#endif
