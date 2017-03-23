/**
 * @file	Feedback.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_FEEDBACK_HPP_
#define MAP_RUNTIME_DAG_FEEDBACK_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Loop; //!< Forward declaration
struct LoopHead; //!< Forward declaration

struct Feedback : public Node
{
	// Internal declarations
	struct Key {
		Key(Feedback *node);
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
	Feedback(Loop *loop, LoopHead *prev);
	Feedback(Loop *loop, Feedback *feed_in, Node *prev);
	Feedback(const Feedback *other, NodeList new_prev_list, NodeList new_back_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Loop* loop() const;
	Node* prev() const;
	bool feedIn() const;
	bool feedOut() const;
	Pattern pattern() const { return SPREAD; }

	//const NodeList& prevList() const;
	//const NodeList& nextList() const;

	// Variables
	Loop *owner_loop;
	bool in_or_out; //!< true / false --> FeedIn / FeedOut type
	Feedback *twin; // linked feedback (i.e. feed_in <--> feed_out)
};

} } // namespace map::detail

#endif
