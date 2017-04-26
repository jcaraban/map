/**
 * @file	Unary.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Local Unary operation (e.g. !,~,cos,sqrt,abs,log,exp,floor)
 */

#ifndef MAP_RUNTIME_DAG_UNARY_HPP_
#define MAP_RUNTIME_DAG_UNARY_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Unary : public Node
{
	// Internal declarations
	struct Content {
		Content(Unary *node);
		bool operator==(const Content& k) const;
		Node *prev;
		UnaryType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, UnaryType type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Unary(const MetaData &meta, Node *prev, UnaryType type);
	Unary(const Unary *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	
	// Spatial
	Pattern pattern() const { return LOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;
	
	// Compute
	void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	UnaryType type; //!< Enum corresponding to the type of unary operation / function
};

} } // namespace map::detail

#endif
