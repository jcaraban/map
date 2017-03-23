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
	struct Key {
		Key(Unary *node);
		bool operator==(const Key& k) const;
		Node *prev;
		UnaryType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, UnaryType type);
	Node* clone(NodeList new_prev_list, NodeList new_back_list);

	// Constructors
	Unary(const MetaData &meta, Node *prev, UnaryType type);
	Unary(const Unary *other, NodeList new_prev_list, NodeList new_back_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Pattern pattern() const { return LOCAL; }

	// Variables
	UnaryType type; //!< Enum corresponding to the type of unary operation / function
};

} } // namespace map::detail

#endif
