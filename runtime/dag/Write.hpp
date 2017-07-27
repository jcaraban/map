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
	struct Content {
		Content(Write *node);
		bool operator==(const Content& k) const;
		Node *prev;
		std::string path;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};
	
	// Factory
	static Node* Factory(Node *prev, std::string file_path);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Write(Node *prev, SharedFile out_file);
	Write(const Write *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string shortName() const;
	std::string longName() const;
	std::string signature() const;
	char classSignature() const;

	// Features
	bool canForward() const { return true; };

	// Compute
	//void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

	// Variables
	//AnyFile out_file;
};

} } // namespace map::detail

#endif
