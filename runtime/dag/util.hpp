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

Pattern isInputOf(const Node *node, const Group *group);

// 

template <typename T>
bool is_included(const T *elem, const std::vector<T*> &list) {
	auto pred = [&](T *e) { return e == elem; }; // address comparison
	return std::find_if(list.begin(),list.end(),pred) != list.end();
}

template <typename T>
std::vector<T*> inner_join(const std::vector<T*> &lhs, const std::vector<T*> &rhs) {
	std::vector<T*> join;
	for (auto left : lhs) for (auto right : rhs) // double for
		if (left == right) {
			join.push_back(left);
			break; // push & break 2nd for
		}
	return join;
}

template <typename T>
std::vector<T*> full_join(const std::vector<T*> &lhs, const std::vector<T*> &rhs) {
	std::vector<T*> join;
	join.reserve(lhs.size()+rhs.size());
	join.insert(join.end(),lhs.begin(),lhs.end());
	join.insert(join.end(),rhs.begin(),rhs.end());
	return join;
}

template <typename T>
std::vector<T*> full_unique_join(const std::vector<T*> &lhs, const std::vector<T*> &rhs) {
	std::vector<T*> join;
	join.reserve(lhs.size()+rhs.size());
	join.insert(join.end(),lhs.begin(),lhs.end());
	for (auto right : rhs)
		if (not is_included(right,join))
			join.push_back(right);
	return join;
}

template <typename T>
std::vector<T*> left_join(const std::vector<T*> &lhs, const std::vector<T*> &rhs) {
	std::vector<T*> join;
	for (auto left : lhs)
		if (!is_included(left,rhs))
			join.push_back(left);
	return join;
}

template <typename T>
void remove_value(const T *value, std::vector<T*> &list) {
	assert( std::find(list.begin(),list.end(),value) != list.end() );
	list.erase(std::remove(list.begin(),list.end(),value),list.end());
}

template <typename T>
int value_position(const T *value, const std::vector<T*> &list) {
	auto it = std::find(list.begin(),list.end(),value);
	assert(it != list.end());
	return std::distance(list.begin(),it);
}

} } // namespace map::detail

#endif
