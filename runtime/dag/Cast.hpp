/**
 * @file	Cast.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a type Cast to another data type (e.g. (int)x, (float)y)
 */

#ifndef MAP_RUNTIME_DAG_CAST_HPP_
#define MAP_RUNTIME_DAG_CAST_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Cast : public Node
{
	// Internal declarations
	struct Key {
		Key(Cast *node);
		bool operator==(const Key& k) const;
		Node *prev;
		DataType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, DataType new_type);
	Node* clone(NodeList new_prev_list);

	// Constructors
	Cast(const MetaData &meta, Node *prev);
	Cast(const Cast *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Pattern pattern() const { return LOCAL; }

	// Variables
	DataType type; //!< Enum corresponding to the type of cast
};

} } // namespace map::detail

#endif
