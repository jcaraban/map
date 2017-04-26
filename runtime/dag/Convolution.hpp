/**
 * @file	Convolution.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Convolution operation with static mask
 *
 * TODO: either add another node for dynamic masks or create a new Dyn-Conv-OP
 */

#ifndef MAP_RUNTIME_DAG_CONVOLUTION_HPP_
#define MAP_RUNTIME_DAG_CONVOLUTION_HPP_

#include "Node.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

struct Convolution : public Node
{
	// Internal declarations
	struct Content {
		Content(Convolution *node);
		bool operator==(const Content& k) const;
		Node *prev;
		Mask smask;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, const Mask &mask);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Convolution(const MetaData &meta, Node *prev, const Mask &mask);
	Convolution(const Convolution *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node* prev() const;
	Mask mask() const;
	
	// Spatial
	Pattern pattern() const { return FOCAL; }
	// const Mask& inputReach(Coord coord) const;
	// const Mask& outputReach(Coord coord) const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	Mask smask; //!< Static mask
};

} } // namespace map::detail

#endif
