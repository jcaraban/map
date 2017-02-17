/**
 * @file	Read.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing an Input Read operation
 */

#ifndef MAP_RUNTIME_DAG_READ_HPP_
#define MAP_RUNTIME_DAG_READ_HPP_

#include "Node.hpp"
#include "IO.hpp"


namespace map { namespace detail {

//typedef std::unique_ptr<IFile> AnyFile;

struct Read : public InputNode
{
	// Internal declarations
	struct Key {
		Key(Read *node);
		bool operator==(const Key& k) const;
		std::string path;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};
	
	// Factory
	static Node* Factory(std::string file_path);
	static Node* Clone(Read *read); // @

	// Constructors & methods
	Read(SharedFile in_file);
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;

	// Variables
	//AnyFile in_file;
};

} } // namespace map::detail

#endif
