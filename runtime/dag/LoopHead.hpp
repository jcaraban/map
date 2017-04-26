/**
 * @file	LoopHead.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_LOOPHEAD_HPP_
#define MAP_RUNTIME_DAG_LOOPHEAD_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct LoopCond; //!< Forward declaration
struct LoopTail; //!< Forward declaration

struct LoopHead : public Node
{
	// Internal declarations
	struct Content {
		Content(LoopHead *node);
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
	LoopHead(const MetaData &meta, Node *prev);
	LoopHead(const LoopHead *other, const std::unordered_map<Node*,Node*> &other_to_this);
	~LoopHead();

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	LoopCond* loop() const;
	Node* prev() const;

	// Spatial
	Pattern pattern() const { return HEAD; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Variables
	LoopCond *owner_loop;
	LoopTail *twin_tail;
};

} } // namespace map::detail

#endif
