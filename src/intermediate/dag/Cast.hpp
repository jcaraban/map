/**
 * @file	Cast.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a type Cast to another data type (e.g. (int)x, (float)y)
 */

#ifndef MAP_RUNTIME_DAG_CAST_HPP_
#define MAP_RUNTIME_DAG_CAST_HPP_

#include "../Node.hpp"


namespace map { namespace detail {

struct Cast : public Node
{
	// Internal declarations
	struct Content {
		Content(Cast *node);
		bool operator==(const Content& k) const;
		Node *prev;
		DataType type;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, DataType new_type);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Cast(const MetaData &meta, Node *prev);
	Cast(const Cast *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	
	// Spatial
	Pattern pattern() const { return LOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;
	
	// Compute
	void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	DataType type; //!< Enum corresponding to the type of cast
};

} } // namespace map::detail

#endif
