/**
 * @file	LhsAccess.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing an LhsAccess operation
 *
 * Note: was and could be named 'Blend', 'Compose', 'Combine'
 */

#ifndef MAP_RUNTIME_DAG_LHSACCESS_HPP_
#define MAP_RUNTIME_DAG_LHSACCESS_HPP_

#include "../Node.hpp"


namespace map { namespace detail {

struct LhsAccess : public Node
{
	// Internal declarations
	struct Content {
		Content(LhsAccess *node);
		bool operator==(const Content& k) const;
		Node *lprev, *rprev;
		Coord coord;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *lhs, Node *rhs, const Coord &coord);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	LhsAccess(const MetaData &meta, Node *lprev, Node *rprev, const Coord &coord);
	LhsAccess(const LhsAccess *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* left() const;
	Node* right() const;
	Coord coord() const;
	
	// Spatial
	Pattern pattern() const { return LOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Variables
	Coord cell_coord; //!< LhsAccess coordinate
};

} } // namespace map::detail

#endif
