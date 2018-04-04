/**
 * @file	Read.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing an Input Read operation
 */

#ifndef MAP_RUNTIME_DAG_READ_HPP_
#define MAP_RUNTIME_DAG_READ_HPP_

#include "../Node.hpp"
#include "IO.hpp"


namespace map { namespace detail {

struct Read : public InputNode
{
	// Internal declarations
	struct Content {
		Content(Read *node);
		bool operator==(const Content& k) const;
		std::string path;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};
	
	// Factory
	static Node* Factory(std::string file_path);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Read(SharedFile in_file);
	Read(const Read *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;

	// Variables
};

} } // namespace map::detail

#endif
