/**
 * @file	Conditional.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Local Conditional operation (i.e. cond ? left : right)
 *
 * TODO: tensorflow features select() and cond().
 * - select() is an element-wise conditional, like this op.
 * - cond() takes a scalar 'pred' and performs true branching
 * If we use a Zonal for 'pred', at runtime one side could be optimized away
 */

#ifndef MAP_RUNTIME_DAG_CONDITIONAL_HPP_
#define MAP_RUNTIME_DAG_CONDITIONAL_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Conditional : public Node
{
	// Internal declarations
	struct Content {
		Content(Conditional *node);
		bool operator==(const Content& k) const;
		Node *cond, *lprev, *rprev;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *cond, Node *lhs, Node *rhs);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Conditional(const MetaData &meta, Node *cond, Node *lprev, Node *rprev);
	Conditional(const Conditional *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* cond() const;
	Node* left() const;
	Node* right() const;
	
	// Spatial
	Pattern pattern() const { return LOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
};

} } // namespace map::detail

#endif
