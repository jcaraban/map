/**
 * @file	Loop.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Symbolic Loop construction
 */

#ifndef MAP_RUNTIME_DAG_LOOP_HPP_
#define MAP_RUNTIME_DAG_LOOP_HPP_

#include "Node.hpp"
#include "LoopCond.hpp"
#include "LoopHead.hpp"
#include "LoopTail.hpp"
#include "Feedback.hpp"


namespace map { namespace detail {

typedef std::vector<LoopHead*> HeadList;
typedef std::vector<LoopTail*> TailList;
typedef std::vector<Feedback*> FeedList;

struct Loop : public Node
{
	// Internal declarations
	struct Key {
		Key(Loop *node);
		bool operator==(const Key& k) const;
		NodeList prev_list;
		NodeList cond_list, body_list;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(NodeList prev_list, Node *cond, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list);
	Node* clone(NodeList new_prev_list);

	// Constructors
	Loop(const MetaData &meta, NodeList prev_list, Node *cond, NodeList body_list, NodeList feed_in_list, NodeList feed_out_list);
	Loop(const Loop *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	
	LoopCond* condition() const;
	const NodeList& bodyList() const;
	const HeadList& headList() const;
	const TailList& tailList() const;
	Pattern pattern() const { return SPREAD; }

	// Variables
	LoopCond *cond_node; //!< Node representing the enter / exit condition
	NodeList body_list; //!< Nodes forming the body of the Symbolic Loop
	
	HeadList head_list; //!< Head nodes acting as inputs for the body nodes
	TailList tail_list; //!< Tail nodes acting as outputs of the loop
	FeedList feed_in_list; //!< Feedback nodes: intersection of head & tail
	FeedList feed_out_list; //!< In/Out feedback nodes are regularly swapped

	NumDim gen_num_dim; //!< Least 'number of dimensions' covering all nodes
	DataSize gen_data_size; //!< Least 'data size' covering all nodes
	BlockSize gen_block_size; //!< Least 'block size' covering all nodes
	NumBlock gen_num_block; //!< Least 'number of blocks' covering all nodes
};

} } // namespace map::detail

#endif
