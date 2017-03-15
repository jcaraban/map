/**
 * @file	Write.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing an Output Write operation
 */

#ifndef MAP_RUNTIME_DAG_WRITE_HPP_
#define MAP_RUNTIME_DAG_WRITE_HPP_

#include "Node.hpp"
#include "IO.hpp"


namespace map { namespace detail {

//typedef std::unique_ptr<IFile> AnyFile;

struct Write : public OutputNode
{
	// Internal declarations
	struct Key {
		Key(Write *node);
		bool operator==(const Key& k) const;
		Node *prev;
		std::string path;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};
	
	// Factory
	static Node* Factory(Node *prev, std::string file_path);
	Node* clone(NodeList new_prev_list);

	// Constructors
	Write(Node *prev, SharedFile out_file);
	Write(const Write *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;

	// Variables
	//AnyFile out_file;
};

} } // namespace map::detail

#endif
