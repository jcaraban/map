/**
 * @file	Checkpoint.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_CHECKPOINT_HPP_
#define MAP_RUNTIME_DAG_CHECKPOINT_HPP_

#include "IO.hpp"


namespace map { namespace detail {

//typedef std::unique_ptr<IFile> AnyFile;

struct Checkpoint : public OutInNode
{
	// Internal declarations
	struct Content {
		Content(Checkpoint *node);
		bool operator==(const Content& k) const;
		Node *prev;
	};
	struct Hash {
		std::size_t operator()(const Content& k) const;
	};
	
	// Factory
	static Node* Factory(Node *prev);
	Node* clone(const std::unordered_map<Node*,Node*> &other_to_this);

	// Constructors
	Checkpoint(Node *prev, SharedFile tmp_file);
	Checkpoint(const Checkpoint *other, const std::unordered_map<Node*,Node*> &other_to_this);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;

	// Compute
	//void computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash);
	void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);
	
	// Variables
	//AnyFile file;
};

} } // namespace map::detail

#endif
