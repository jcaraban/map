/**
 * @file	Group.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_GROUP_HPP_
#define MAP_RUNTIME_DAG_GROUP_HPP_

#include "../Pattern.hpp"
#include <unordered_set>
#include <memory>


namespace map { namespace detail {

struct Node; //!< Forward declaration
struct Group; //!< Forward declaration
struct Task; // Forward declaration

typedef std::vector<Node*> NodeList;
typedef std::vector<std::unique_ptr<Group>> OwnerGroupList;
typedef std::vector<Group*> GroupList;
typedef std::vector<Pattern> PatternList;

/*
 * Class Group
 */
struct Group {
  // Constructors
	Group();
	Group(Pattern pattern);

  // Methods
	const NodeList& nodeList() const;
	const NodeList& inputList() const;
	const NodeList& outputList() const;
	const GroupList& prevList() const;
	const GroupList& nextList() const;
	const GroupList& backList() const;
	const GroupList& forwList() const;

	NumDim numdim() const;
	const DataSize& datasize() const;
	const BlockSize& blocksize() const;
	const NumBlock& numblock() const;

	void addNode(Node *node);
	void removeNode(Node *node);

	void addInputNode(Node *node);
	void removeInputNode(Node *node);

	void addOutputNode(Node *node);
	void removeOutputNode(Node *node);

	void addAutoNode(Node *node);
	void removeAutoNode(Node *node);

	void addPrev(Group *group, Pattern pattern);
	void removePrev(Group *group);
	bool isPrev(const Group *group) const;

	void addNext(Group *group, Pattern pattern);
	void removeNext(Group *group);
	bool isNext(const Group *group) const;

	void addBack(Group *group, Pattern pattern=NONE_PAT); // @
	void removeBack(Group *group);
	void addForw(Group *group, Pattern pattern=NONE_PAT); // @
	void removeForw(Group *group);

	void updateAttributes() const;
	void updatePrevious() const;

	Pattern& prevPattern(GroupList::const_iterator i);
	Pattern& prevPattern(Group *prev);
	Pattern& nextPattern(GroupList::const_iterator i);
	Pattern& nextPattern(Group *next);
	Pattern& pattern();
	const Pattern& pattern() const;

	std::string getName() const;
	std::string signature() const;
	char classSignature() const;

  // Variables
	static int id_count; // Static counter to give unique ids
	int id; //!< Unique id of the group
	Task *task; //!< Task produced from this Group
	
	NodeList node_list; //!< List of all nodes fused into the group
	NodeList in_list; //!< List of input-nodes into the group
	NodeList out_list; //!< List of output-nodes into the group
	
	GroupList prev_list; //!< List of prev groups, whose nodes the nodes on this group depends on
	GroupList next_list; //!< List of next groups, whose nodes depend on the nodes of this group
	GroupList back_list; //!< List of back groups
	GroupList forw_list; //!< List of forward groups
	
	PatternList prev_pat; //!< Patterns of the respective prev groups as this group sees them (i.e. only considering the related nodes)
	PatternList next_pat; //!< 	"		"		"		" next groups 	"		"		"		"		"		"	
	
	Pattern gen_pattern; //!< General pattern, composed by the addition of the patterns of all the nodes
	mutable NumDim gen_num_dim; //!< Least 'number of dimensions' covering all nodes
	mutable DataSize gen_data_size; //!< Least 'data size' covering all nodes
	mutable BlockSize gen_block_size; //!< Least 'block size' covering all nodes
	mutable NumBlock gen_num_block; //!< Least 'number of blocks' covering all nodes
	mutable bool gen_outdated; //!< Stores if the general attributes are outdated

	mutable std::unordered_set<const Group*> prev_hash; //!< LookUp container for prev of prev groups
	mutable bool prev_outdated;  //!< Stores if the 'prev_hash' structure is outdated
};

} } // namespace map::detail

#endif
