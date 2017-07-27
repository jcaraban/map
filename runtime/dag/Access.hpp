/**
 * @file	Access.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing an Access
 *
 * Note: was and could be named 'Window'
 */

#ifndef MAP_RUNTIME_DAG_ACCESS_HPP_
#define MAP_RUNTIME_DAG_ACCESS_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Access : public Node
{
	// Internal declarations
	struct Content {
		Content(Access *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Coord coord;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, const Coord &coord);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Access(const MetaData &meta, Node *prev, const Coord &coord);
	Access(const Access *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Coord coord() const;
	
	// Spatial
	Pattern pattern() const { return GLOBAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Variables
	Coord cell_coord; //!< Access coordinate
};

} } // namespace map::detail

#endif
