/**
 * @file	IO.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "IO.hpp"


namespace map { namespace detail {

/******
   IO
 ******/

IONode::IONode() { }

IONode::IONode(SharedFile file, InputNodeFlag not_used)
	: Node(file->getMetaData())
	, io_file(file)
{ }

IONode::IONode(SharedFile file, OutputNodeFlag not_used)
	: Node() // OutputNode shares the unique id
	, io_file(file)
{ }

IONode::IONode(const IONode *other, NodeList new_prev)
	: Node(other,new_prev)
	, io_file(other->io_file)
{ }

IFile* IONode::file() {
	return io_file.get();
}

const IFile* IONode::file() const {
	return io_file.get();
}

/******
   In
 ******/

InputNode::InputNode() { }

InputNode::InputNode(SharedFile file)
	: IONode(file,InputNodeFlag())
{ }

bool InputNode::isInput() const {
	return true;
}
/*
std::string InputNode::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += file()->getNumDim().toString();
	sign += file()->getDataType().toString();
	return sign;
}
*/
/*******
   Out
 *******/

OutputNode::OutputNode() { }

OutputNode::OutputNode(Node *prev, SharedFile file)
	: IONode(file,OutputNodeFlag())
{
	id = prev->id; // OutputNodes shares id with the node that is 'backing' in disk
	ref = 0;
	meta = prev->metadata();
	stats = prev->datastats();

	prev_list.resize(1);
	prev_list[0] = prev;
	prev->addNext(this);
}

bool OutputNode::isOutput() const {
	return true;
}
/*
std::string OutputNode::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	return sign;
}
*//*
Node*& OutputNode::prev() {
	return prev_list[0];
}
*/
Node* OutputNode::prev() const {
	return prev_list[0];
}

/*********
   OutIn
 *********/

OutInNode::OutInNode() { }

OutInNode::OutInNode(Node *prev, SharedFile file)
	: IONode(file,OutputNodeFlag())
	, InputNode()
	, OutputNode(prev,file)
{
	meta.stream_dir = IO;
	filled = false;
}

const NodeList& OutInNode::prevList() const {
	return (!filled) ? prev_list : empty_list;
}

const NodeList& OutInNode::nextList() const {
	return (filled) ? next_list : empty_list;
}

bool OutInNode::isInput() const {
	return filled;
}

bool OutInNode::isOutput() const {
	return !filled;
}

void OutInNode::setFilled() {
	assert(!filled);
	filled = true;
}

void OutInNode::unsetFilled() {
	assert(filled);
	filled = false;
}

} } // namespace map::detail
