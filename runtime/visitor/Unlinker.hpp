/**
 * @file	Unlinker.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Backward visitor that unlinks non-referenced nodes and returns the list
 */

#ifndef MAP_RUNTIME_VISITOR_UNLINKER_HPP_
#define MAP_RUNTIME_VISITOR_UNLINKER_HPP_

#include "Visitor.hpp"
#include <queue>


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Return a list only with the unlinked nodes
 */
struct Unlinker : public Visitor
{
  // constructor and main function
	Unlinker();
	NodeList unlink(const OwnerNodeList &node_list);

  // methods
	void clear();
	bool isIsolated(Node* node);
  	void setIsolated(Node* node);
  	bool isLiberated(Node* node);
  	void setLiberated(Node* node);

  // visit
	DECLARE_VISIT(Merge)
	DECLARE_VISIT(Switch)

  // vars
	std::unordered_set<Node*> isolated;
	std::unordered_set<Node*> liberated;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
