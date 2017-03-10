/**
 * @file	LoopHead.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_LOOPHEAD_HPP_
#define MAP_RUNTIME_DAG_LOOPHEAD_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Loop; //!< Forward declaration

struct LoopHead : public Node
{
	// Internal declarations
	struct Key {
		Key(LoopHead *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Loop *loop;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};
	
	// Constructors & methods
	LoopHead(Loop *loop, Node *prev);
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
