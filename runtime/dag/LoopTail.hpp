/**
 * @file	LoopTail.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_LOOPTAIL_HPP_
#define MAP_RUNTIME_DAG_LOOPTAIL_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Loop; //!< Forward declaration

struct LoopTail : public Node
{
	// Internal declarations
	struct Key {
		Key(LoopTail *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Loop *loop;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};
	
	// Factory
	Node* clone(NodeList new_prev_list, NodeList new_back_list);

	// Constructors
	LoopTail(Loop *loop, Node *prev);
	LoopTail(const LoopTail *other, NodeList new_prev_list, NodeList new_back_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Loop* loop() const;
	Node* prev() const;
	Pattern pattern() const { return SPREAD; }

	// Variables
	Loop *owner_loop;
};

} } // namespace map::detail

#endif
