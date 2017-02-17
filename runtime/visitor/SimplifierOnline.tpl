/** 
 * @file    SimplifierOnline.tpl 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_RUNTIME_VISITOR_SIMPLIFIER_ONLINE_TPL_
#define MAP_RUNTIME_VISITOR_SIMPLIFIER_ONLINE_TPL_


namespace map { namespace detail {

template <typename T>
void SimplifierOnline::helper(T *node, std::unordered_map<typename T::Key,T*,typename T::Hash> &map) {
	typename T::Key key(node);
	auto i = map.find(key);
	if (i == map.end()) // No other similar Node found
	{
		// Keeps track of the node in a std:map structure
		map.insert( std::pair<typename T::Key,T*>(key,node) );

		// This is the first node, therefore the original
		orig = node;
	}
	else  // Another similar Node already exists
	{
		// Removes all links from prev-nodes to the repeated node
		for (auto j : node->prevList())
			j->removeNext(node);
		
		// Deletes the repeated node, which is last in 'node_list'
		node_list.pop_back();

		// Reverts Node::id_count, // @ I'd be fired for this
		Node::id_count--;

		// The existing node was the original
		orig = i->second;
	}
}

template <typename T>
void SimplifierOnline::drop_helper(T *node, std::unordered_map<typename T::Key,T*,typename T::Hash> &map) {
	typename T::Key key(node);
	auto i = map.find(key);
	assert(i != map.end());
	map.erase(i);
}

} } // namespace map::detail

#endif
