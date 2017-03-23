/**
 * @file	Convolution.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Focal Convolution operation with static mask
 *
 * TODO: add another prev_node for when the mask is dynamic (i.e. the mask is another raster) 
 */

#ifndef MAP_RUNTIME_DAG_CONVOLUTION_HPP_
#define MAP_RUNTIME_DAG_CONVOLUTION_HPP_

#include "Node.hpp"
#include "../../util/Mask.hpp"


namespace map { namespace detail {

struct Convolution : public Node
{
	// Internal declarations
	struct Key {
		Key(Convolution *node);
		bool operator==(const Key& k) const;
		Node *prev;
		Mask smask;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(Node *prev, const Mask &mask);
	Node* clone(NodeList new_prev_list, NodeList new_back_list);

	// Constructors
	Convolution(const MetaData &meta, Node *prev, const Mask &mask);
	Convolution(const Convolution *other, NodeList new_prev_list, NodeList new_back_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	//Node*& prev();
	Node* prev() const;
	Mask mask() const;
	Pattern pattern() const { return FOCAL; }
	BlockSize halo() const;

	// Variables
	Mask smask; //!< Static mask
};

} } // namespace map::detail

#endif
