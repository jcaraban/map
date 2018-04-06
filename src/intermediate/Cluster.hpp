/**
 * @file	Cluster.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: change name from 'Cluster' of nodes to 'Cluster' of nodes
 */

#ifndef MAP_RUNTIME_DAG_GROUP_HPP_
#define MAP_RUNTIME_DAG_GROUP_HPP_

#include "../Pattern.hpp"
#include "../file/MetaData.hpp"
#include <unordered_set>
#include <memory>


namespace map { namespace detail {

struct Node; //!< Forward declaration
struct Cluster; //!< Forward declaration
struct Task; // Forward declaration

typedef std::vector<Node*> NodeList;
typedef std::vector<std::unique_ptr<Cluster>> OwnerClusterList;
typedef std::vector<Cluster*> ClusterList;
typedef std::vector<Pattern> PatternList;

/*
 * Class Cluster
 */
struct Cluster {
  // Constructors
	Cluster();
	//Cluster(Pattern pattern);

  // Methods
	const NodeList& nodeList() const;
	const NodeList& inputList() const;
	const NodeList& outputList() const;
	const ClusterList& prevList() const;
	const ClusterList& nextList() const;
	const ClusterList& backList() const;
	const ClusterList& forwList() const;

	NumDim numdim() const;
	const DataSize& datasize() const;
	const BlockSize& blocksize() const;
	const NumBlock& numblock() const;
	const GroupSize& groupsize() const;
	const NumGroup& numgroup() const;

	void addNode(Node *node);
	void removeNode(Node *node);

	void addInputNode(Node *node);
	void removeInputNode(Node *node);

	void addOutputNode(Node *node);
	void removeOutputNode(Node *node);

	void addAutoNode(Node *node);
	void removeAutoNode(Node *node);

	void addPrev(Cluster *cluster, Pattern pattern);
	void removePrev(Cluster *cluster);
	bool isPrev(const Cluster *cluster) const;

	void addNext(Cluster *cluster, Pattern pattern);
	void removeNext(Cluster *cluster);
	bool isNext(const Cluster *cluster) const;

	void addBack(Cluster *cluster, Pattern pattern=NONE_PATTERN); // @
	void removeBack(Cluster *cluster);
	void addForw(Cluster *cluster, Pattern pattern=NONE_PATTERN); // @
	void removeForw(Cluster *cluster);

	void updateAttributes() const;
	void updatePrevious() const;

	Pattern& prevPattern(ClusterList::const_iterator i);
	Pattern& prevPattern(Cluster *prev);
	Pattern& nextPattern(ClusterList::const_iterator i);
	Pattern& nextPattern(Cluster *next);
	Pattern& pattern();
	const Pattern& pattern() const;

	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;

  // Variables
	int id; //!< Unique id of the cluster
	Task *task; //!< Task produced from this Cluster
	
	NodeList node_list; //!< List of all nodes fused into the cluster
	NodeList in_list; //!< List of input-nodes into the cluster
	NodeList out_list; //!< List of output-nodes into the cluster
	
	ClusterList prev_list; //!< List of prev clusters, whose nodes the nodes on this cluster depends on
	ClusterList next_list; //!< List of next clusters, whose nodes depend on the nodes of this cluster
	ClusterList back_list; //!< List of back clusters
	ClusterList forw_list; //!< List of forward clusters
	
	PatternList prev_pat; //!< Patterns of the respective prev clusters as this cluster sees them (i.e. only considering the related nodes)
	PatternList next_pat; //!< 	"		"		"		" next clusters 	"		"		"		"		"		"	
	
	Pattern gen_pattern; //!< General pattern, composed by the addition of the patterns of all the nodes
	mutable DataShape gen_shape; //! Least 'shape' encompassing all nodes
	mutable bool gen_outdated; //!< Stores if the general attributes are outdated

	mutable std::unordered_set<const Cluster*> prev_hash; //!< LookUp container for prev of prev clusters
	mutable bool prev_outdated;  //!< Stores if the 'prev_hash' structure is outdated
};

} } // namespace map::detail

#endif
