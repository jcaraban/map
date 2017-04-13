/**
 * @file	util.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Utilities related to Node
 */

#ifndef MAP_RUNTIME_DAG_UTIL_HPP_
#define MAP_RUNTIME_DAG_UTIL_HPP_

#include "Node.hpp"
#include "Group.hpp"


namespace map { namespace detail {

//struct Node; // forward declaration
//struct Group; // forward declaration

struct node_id_less {
	bool operator()(const Node *lhs, const Node *rhs);
};
struct node_id_equal {
	bool operator()(const Node *lhs, const Node *rhs);
};
struct node_id_greater {
	bool operator()(const Node *lhs, const Node *rhs);
};

bool is_included(const Node *node, const NodeList &list);
bool is_included(const Group *group, const GroupList &list);

NodeList inner_join(const NodeList &lhs, const NodeList &rhs);
NodeList full_join(const NodeList &lhs, const NodeList &rhs);
NodeList left_join(const NodeList &lhs, const NodeList &rhs);

template <typename T>
void remove_value(const T *value, std::vector<T*> &list) {
	assert( std::find(list.begin(),list.end(),value) != list.end() );
	list.erase(std::remove(list.begin(),list.end(),value),list.end());
}

int value_position(const Node *node, const NodeList &list);

Pattern isInputOf(const Node *node, const Group *group);

} } // namespace map::detail

#endif
