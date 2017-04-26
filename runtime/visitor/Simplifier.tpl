/** 
 * @file    Simplifier.tpl 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_RUNTIME_VISITOR_SIMPLIFIER_TPL_
#define MAP_RUNTIME_VISITOR_SIMPLIFIER_TPL_


namespace map { namespace detail {

template <typename T>
void Simplifier::helper(T *node, std::unordered_map<typename T::Content,T*,typename T::Hash> &map) {
	typename T::Content key(node);
	auto i = map.find(key);
	if (i == map.end()) // No other similar Node found
	{
		// Keeps track of the node in a std:map structure
		map.insert( std::pair<typename T::Content,T*>(key,node) );

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
void Simplifier::drop_helper(T *node, std::unordered_map<typename T::Content,T*,typename T::Hash> &map) {
	typename T::Content key(node);
	map.erase(key); // @ reconsider the whole 'drop' idea
}

} } // namespace map::detail

#endif
