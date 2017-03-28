/**
 * @file	Fusioner.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the DAG that fuses Nodes into Groups
 *
 * TODO: fusioner should not touch 'Runtime::group_list' but return a Structure from fuse()
 *       the struct should contain the 'group_list' and a 'group_list_of'
 *
 */

#ifndef MAP_RUNTIME_VISITOR_FUSIONER_HPP_
#define MAP_RUNTIME_VISITOR_FUSIONER_HPP_

#include "Visitor.hpp"
#include <unordered_set>
#include <unordered_map>


namespace map { namespace detail {

typedef std::vector<std::unique_ptr<Group>> OwnerGroupList; // forward declaration

/*
 *
 */
struct Fusioner
{
  // constructor and main function
	Fusioner(OwnerGroupList& group_list);
	void fuse(NodeList list);

  // methods
	void clear();

	/*
	 * Inserts a new group in 'group_list' and returns it
	 */
	Group* newGroup();

	/*
	 * Deletes 'group' from the list of groups
	 */
	void removeGroup(Group *group);

	/*
	 * returns true when 'top' and 'bot' can be pipe-fused following the top-bot direcction
	 */
	bool canPipeFuse(Group *top, Group *bot);

	/*
	 * returns true when 'left' and 'right' can be flat-fused
	 */
	bool canFlatFuse(Group *left, Group *right);

	/*
	 * The content of 'bot' is transferred to 'top' and it's erased
	 * Links of all involved 'prev' and 'next' nodes are updated
	 * Only valid for groups with producer-consumer data-flow relations
	 */
	Group* pipeFuseGroup(Group *&top, Group *&bot);	

	/*
	 * The content of 'right' is transferred to 'left' and it's erased
	 * Links of all involved 'prev' and 'next' nodes are updated
	 * Only valid for groups with consumer-consumer data-flow relations
	 */
	Group* flatFuseGroup(Group *&left, Group *&right);

	/*
	 * The content of 'other' is transferred to 'one' and it's erased
	 * Valid for any type of data-flow relations and algorithmic pattern
	 * NB: use carefully, fusing incompatible groups will result in errors
	 */
	Group* freeFuseGroup(Group *&one, Group *&other);

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
	void process(Group *group);
	void processBU(Group *group); // @

	void processLoop(Node *node); // @

	/*
	 * For every block with only input/free-nodes, their content is forwarded
	 * to those next-blocks depending on them and they are removed
	 */
	void forwarding(std::function<bool(Node*)> pred);

	/*
	 * Marks as Input/Output all those nodes in the group boundary that link to other groups
	 */
	void linking();

	/*
	 * Sorts group in order of dependency. Sorts nodes within groups in order of id
	 */
	void sorting();

	/*
	 * Topological sort for the Groups DAG
	 */
	void toposort(Group *group);

	/*
	 *
	 */
	void print();

  // vars
	OwnerGroupList& group_list; //!< Aggregation, Fusioner does not own the data
	std::unordered_map<Node*,GroupList> group_list_of; //!< For each node, stores its groups
	
	std::unordered_set<Group*> visited; // Remembers what groups have been previously visited
	GroupList sorted_group_list; //!< Stores the groups in topological order
};

} } // namespace map::detail

#endif
