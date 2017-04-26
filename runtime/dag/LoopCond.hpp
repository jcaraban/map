/**
 * @file	LoopCond.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Symbolic Loop construction
 *
 * TODO: add a scope() function that walks back/forward to find the 'body' nodes
 */

#ifndef MAP_RUNTIME_DAG_LOOP_HPP_
#define MAP_RUNTIME_DAG_LOOP_HPP_

#include "Node.hpp"
#include "LoopHead.hpp"
#include "LoopTail.hpp"
#include "Merge.hpp"
#include "Switch.hpp"
#include "Identity.hpp"


namespace map { namespace detail {

typedef std::vector<LoopHead*> HeadList;
typedef std::vector<LoopTail*> TailList;
typedef std::vector<Merge*> MergeList;
typedef std::vector<Switch*> SwitchList;

struct LoopCond : public Node
{
	// Internal declarations
	struct Content {
		Content(LoopCond *node);
		bool operator==(const Content& k) const;
		Node *prev;
		HeadList head_list;
		TailList tail_list;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	LoopCond(const MetaData &meta, Node *prev);
	LoopCond(const LoopCond *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	
	const HeadList& headList() const;
	const TailList& tailList() const;
	Node* prev() const;
	
	// Spatial
	Pattern pattern() const { return LOOP; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Variables
	HeadList head_list; //!< Head nodes acting as inputs for the body
	TailList tail_list; //!< Tail nodes acting as outputs of the loop
};

} } // namespace map::detail

#endif
