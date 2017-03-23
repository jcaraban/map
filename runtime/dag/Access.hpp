/**
 * @file	Access.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing an Access
 *
 * Note: was and could be named 'Window'
 * TODO: @@ should be ZONAL ? because it's accessed from anywhere
 */

#ifndef MAP_RUNTIME_DAG_ACCESS_HPP_
#define MAP_RUNTIME_DAG_ACCESS_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Access : public Node
{
	// Internal declarations
	struct Key {
		Key(Access *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Coord coord;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg, const Coord &coord);
	Node* clone(NodeList new_prev_list, NodeList new_back_list);

	// Constructors
	Access(const MetaData &meta, Node *prev, const Coord &coord);
	Access(const Access *other, NodeList new_prev_list, NodeList new_back_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Coord coord() const;
	Pattern pattern() const { return LOCAL; }

	// Variables
	Coord cell_coord; //!< Access coordinate
};

} } // namespace map::detail

#endif
