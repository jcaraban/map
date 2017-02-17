/**
 * @file	IO.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_DAG_IO_HPP_
#define MAP_RUNTIME_DAG_IO_HPP_

#include "Node.hpp"
#include "../../file/File.hpp"


namespace map { namespace detail {

typedef std::shared_ptr<IFile> SharedFile;
struct InputNodeFlag { };
struct OutputNodeFlag { };

struct IONode : public Node
{
	IONode();
	IONode(SharedFile file, InputNodeFlag not_used);
	IONode(SharedFile file, OutputNodeFlag not_used);

	IFile* file();
	const IFile* file() const;

  // vars
	SharedFile io_file; //!< File serving as input or output
};

struct InputNode : public virtual IONode
{
	InputNode();
	InputNode(SharedFile file);

	bool isInput() const;
	//std::string signature() const;
	//char classSignature() const;
};

struct OutputNode : public virtual IONode
{
	OutputNode();
	OutputNode(Node *prev, SharedFile file);

	bool isOutput() const;
	//std::string signature() const;
	//char classSignature() const;
	//Node*& prev();
	Node* prev() const;
};

struct InOutNode {
	static_assert(true,"In-Out node not allowed, sort of goes against the actual model");
};

struct OutInNode : public InputNode, public OutputNode
{
	OutInNode(Node *prev, SharedFile file);

	const NodeList& prevList() const;
	const NodeList& nextList() const;
	bool isInput() const;
	bool isOutput() const;
	void setFilled();
	void unsetFilled();

  // vars
	mutable bool filled;
	NodeList empty_list; //!< @
};

} } // namespace map::detail

#endif
