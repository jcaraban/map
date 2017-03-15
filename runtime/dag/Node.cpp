/**
 * @file	node.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Node.hpp"
#include "util.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

int Node::id_count;

Node::Node()
	: id(-1)
	, ref(0)
	, prev_list()
	, next_list()
	, back_list()
	, forw_list()
	, meta()
	, stats()
	, value()
{ }

Node::Node(const MetaData &meta)
	: id(id_count++)
	, ref(0)
	, prev_list()
	, next_list()
	, back_list()
	, forw_list()
	, meta(meta)
	, stats()
	, value()
{ }

Node::Node(const Node *other, NodeList new_prev)
	: id(other->id)
	, ref(0)
	, prev_list()
	, next_list()
	, back_list()
	, forw_list()
	, meta(other->meta)
	, stats(other->stats)
	, value(other->value)
{
	for (auto prev : new_prev)
		prev->addNext(this);
	this->prev_list = new_prev;
}

Node::~Node() {
	assert(ref == 0);
}

void Node::acceptPrev(Visitor *visitor) {
	for (auto &prev : prevList())
		prev->accept(visitor);
}

void Node::acceptNext(Visitor *visitor) {
	for (auto &next : nextList())
		next->accept(visitor);
}

void Node::increaseRef() {
	ref++;
}

void Node::decreaseRef() {
	ref--;
	assert(ref >= 0);
	assert(ref >= next_list.size());
}

const NodeList& Node::prevList() const {
	return prev_list;
}

const NodeList& Node::nextList() const {
	return next_list;
}

const NodeList& Node::backList() const {
	return back_list;
}

const NodeList& Node::forwList() const {
	return forw_list;
}

const MetaData& Node::metadata() const {
	return meta;
}

void Node::updatePrev(Node *old_node, Node *new_node) {
	auto it = std::find(prev_list.begin(),prev_list.end(),old_node);
	assert(it != prev_list.end());
	Runtime::getInstance().removeNode(this); // @
	*it = new_node;
	Runtime::getInstance().updateNode(this); // @
	// @@ ... continue ... rethink remove/update
}

void Node::updateNext(Node *old_node, Node *new_node) {
	auto it = std::find(next_list.begin(),next_list.end(),old_node);
	assert(it != next_list.end());
	*it = new_node;
}

void Node::addPrev(Node *node) {
	prev_list.push_back(node);
}

void Node::addNext(Node *node) {
	next_list.push_back(node);
	increaseRef();
}

void Node::addBack(Node *node) {
	back_list.push_back(node);
	increaseRef();
}

void Node::addForw(Node *node) {
	forw_list.push_back(node);
}

void Node::removeNext(Node *node) {
	auto it = std::find(next_list.begin(),next_list.end(),node);
	assert(it != next_list.end());
	next_list.erase(it);
	decreaseRef();
}

Pattern Node::pattern() const {
	return FREE;
}

bool Node::isInput() const {
	return false;
}

bool Node::isOutput() const {
	return false;
}

BlockSize Node::halo() const {
	return BlockSize{0,0};//,0,0};
}

StreamDir Node::streamdir() const {
	return metadata().getStreamDir();
}

DataType Node::datatype() const {
	return metadata().getDataType();
}

NumDim Node::numdim() const {
	return metadata().getNumDim();
}

MemOrder Node::memorder() const {
	return metadata().getMemOrder();
}

const DataSize& Node::datasize() const {
	return metadata().getDataSize();
}

const BlockSize& Node::blocksize() const {
	return metadata().getBlockSize();
}

const NumBlock& Node::numblock() const {
	return metadata().getNumBlock();
}

const DataStats& Node::datastats() const {
	return stats;
}

} } // namespace map::detail
