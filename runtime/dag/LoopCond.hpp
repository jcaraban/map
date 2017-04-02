/**
 * @file	LoopCond.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Symbolic Loop construction
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
	struct Key {
		Key(LoopCond *node);
		bool operator==(const Key& k) const;
		//NodeList prev_list, body_list;
		Node *prev;
		HeadList head_list;
		TailList tail_list;
		//MergeList merge_list;
		//SwitchList switch_list;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	//static Node* Factory(NodeList prev_list, Node *cond, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list);
	static Node* Factory(Node *prev);
	Node* clone(std::unordered_map<Node*,Node*> other_to_this);

	// Constructors
	//LoopCond(const MetaData &meta, NodeList prev_list, Node *cond, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list);
	LoopCond(const MetaData &meta, Node *prev);
	LoopCond(const LoopCond *other, std::unordered_map<Node*,Node*> other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	
	//const NodeList& bodyList() const;
	const HeadList& headList() const;
	const TailList& tailList() const;
	//const MergeList& mergeList() const;
	//const SwitchList& switchList() const;
	Node* prev() const;
	Pattern pattern() const { return FREE; }

	// Variables
	//NodeList body_list; //!< Nodes forming the body of the Symbolic Loop
	HeadList head_list; //!< Head nodes acting as inputs for the body
	TailList tail_list; //!< Tail nodes acting as outputs of the loop
	//MergeList merge_list; //!< Merge nodes acting as phi functions
	//SwitchList switch_list; //!< Switch nodes acting as branches
	//int cond_index; //!< Marks the Merge node feeding the condition
};

} } // namespace map::detail

#endif
