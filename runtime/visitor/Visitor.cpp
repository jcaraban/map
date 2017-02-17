/**
 * @file	Visitor.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Visitor.hpp"


namespace map { namespace detail {

bool Visitor::wasVisited(Node *node) {
	return visited.find(node) != visited.end();
}

void Visitor::setVisited(Node *node) {
	visited.insert(node);
}

} } // namespace map::detail
