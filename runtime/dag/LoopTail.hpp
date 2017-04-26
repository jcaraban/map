/**
 * @file	LoopTail.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_LOOPTAIL_HPP_
#define MAP_RUNTIME_DAG_LOOPTAIL_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct LoopCond; //!< Forward declaration
struct LoopHead; //!< Forward declaration

struct LoopTail : public Node
{
	// Internal declarations
	struct Content {
		Content(LoopTail *node);
		bool operator==(const Content& k) const;
		Node *prev;
		LoopCond *loop;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};
	
	// Factory
	static Node* Factory(Node *prev);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	LoopTail(const MetaData &meta, Node *prev);
	LoopTail(const LoopTail *other, const std::unordered_map<Node*,Node*> &other_to_this);
	~LoopTail();

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	LoopCond* loop() const;
	Node* prev() const;

	// Spatial
	Pattern pattern() const { return TAIL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Variables
	LoopCond *owner_loop;
	LoopHead *twin_head;
};

} } // namespace map::detail

#endif
