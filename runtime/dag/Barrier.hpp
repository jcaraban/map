/**
 * @fizle	Barrier.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node that imposes a barrier to fusion. Just used to test things.
 */

#ifndef MAP_RUNTIME_DAG_BARRIER_HPP_
#define MAP_RUNTIME_DAG_BARRIER_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Barrier : public Node
{
	// Internal declarations
	struct Key {
		Key(Barrier *node);
		bool operator==(const Key& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *arg);
	Node* clone(NodeList new_prev_list);

	// Constructors
	Barrier(const MetaData &meta, Node *prev);
	Barrier(const Barrier *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Pattern pattern() const { return BARRIER; }

	// Variables
};

} } // namespace map::detail

#endif
