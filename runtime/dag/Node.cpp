/**
 * @file	node.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Node.hpp"
#include "util.hpp"


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

Node::Node(const Node *other, const std::unordered_map<Node*,Node*> &other_to_this)
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
	for (auto other_prev : other->prevList()) {
		Node *this_prev = other_to_this.find(other_prev)->second;
		this_prev->addNext(this);
		this->addPrev(this_prev);
	}

	for (auto other_back : other->backList()) {
		Node *this_back = other_to_this.find(other_back)->second;
		this_back->addForw(this);
		this->addBack(this_back);
	}
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
	*it = new_node;
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

void Node::removeBack(Node *node) {
	auto it = std::find(back_list.begin(),back_list.end(),node);
	assert(it != back_list.end());
	back_list.erase(it);
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

void Node::computeScalar(std::unordered_map<Key,VariantType,key_hash> &hash) {
	assert(!"computeScalar() not implemented in derived class");
}

void Node::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	assert(!"computeFixed() not implemented in derived class");
}

} } // namespace map::detail
