/**
 * @file	Scalar.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing an Output Scalar operation (returns the scalar value to C++/Python)
 */

#ifndef MAP_RUNTIME_DAG_SCALAR_HPP_
#define MAP_RUNTIME_DAG_SCALAR_HPP_

#include "Node.hpp"
#include "IO.hpp"
#include "../../file/scalar.hpp"


namespace map { namespace detail {

//typedef std::unique_ptr< File<scalar> > ScaFile;

struct Scalar : public OutputNode
{
	// Internal declarations
	struct Key {
		Key(Scalar *node);
		bool operator==(const Key& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};
	
	// Factory
	static Node* Factory(Node *arg);
	static Node* Clone(Scalar *scalar, Node *prev); // @

	// Constructors & methods
	Scalar(Node *prev, SharedFile sca_file);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	VariantType value();

	// Variables
	//ScaFile sca_file;
};

} } // namespace map::detail

#endif
