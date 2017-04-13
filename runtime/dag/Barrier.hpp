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
	struct Content {
		Content(Barrier *node);
		bool operator==(const Content& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *arg);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Barrier(const MetaData &meta, Node *prev);
	Barrier(const Barrier *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Pattern pattern() const { return GLOBAL; }

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);
	
	// Variables
};

} } // namespace map::detail

#endif
