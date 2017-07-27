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
{
	this->file = file; // @ pass 'file' in the initializer list?
}

IONode::IONode(SharedFile file, OutputNodeFlag not_used)
	: Node() // OutputNode shares the unique id
{
	this->file = file; // @ pass 'file' in the initializer list?
}

IONode::IONode(const IONode *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->file = other->file;
}

IONode::~IONode() { }

//SharedFile IONode::getFile() {
//	return file;
//}

const SharedFile IONode::getFile() const {
	return file;
}

/******
   In
 ******/

InputNode::InputNode() { }

InputNode::InputNode(SharedFile file)
	: IONode(file,InputNodeFlag())
{
	meta.stream_dir = IN;
	stats = file->getDataStats();

	this->in_spatial_reach = Mask(); // Free
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

bool InputNode::isInput() const {
	return true;
}

/*******
   Out
 *******/

OutputNode::OutputNode() { }

OutputNode::OutputNode(Node *prev, SharedFile file)
	: IONode(file,OutputNodeFlag())
{
	id = prev->id; // shares id with the node being output
	ref = 0;
	meta = prev->metadata();
	
	meta.stream_dir = OUT;
	stats = prev->datastats();

	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true); // @ shall be empty ?
}

OutputNode::~OutputNode() {
	getFile()->setDataStats(stats);
}

bool OutputNode::isOutput() const {
	return true;
}

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
