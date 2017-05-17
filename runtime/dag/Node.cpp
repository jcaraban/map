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
	, spatial_pattern()
	, in_spatial_reach()
	, out_spatial_reach()
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
	, spatial_pattern()
	, in_spatial_reach()
	, out_spatial_reach()
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
	, spatial_pattern(other->spatial_pattern)
	, in_spatial_reach(other->in_spatial_reach)
	, out_spatial_reach(other->out_spatial_reach)
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

bool Node::isInput() const {
	return false;
}

bool Node::isOutput() const {
	return false;
}

bool Node::isReduction() const {
	return inputReach().numdim().toInt() > outputReach().numdim().toInt();
}

bool Node::canForward() const {
	return false;
}

Pattern Node::pattern() const {
	return spatial_pattern;
}

const Mask& Node::inputReach(Coord coord) const {
	return in_spatial_reach;
}

//const Mask& Node::intraReach(Coord coord) const {
//	return intra_spatial_reach;
//}

const Mask& Node::outputReach(Coord coord) const {
	return out_spatial_reach;
}

HoldType Node::holdtype(Coord coord) {
	if (prod(blocksize()) == 1)
		return HOLD_1;
	//else if (prod(groupsize()) == 1)
	//	return HOLD_2;
	else if (numdim() == D2 && all(in_range(coord,numblock())))
		return HOLD_N;
	else if (any(not in_range(coord,numblock())))
		return HOLD_0;
	else
		assert(0);
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

const GroupSize& Node::groupsize() const {
	return metadata().getGroupSize();
}

const NumGroup& Node::numgroup() const {
	return metadata().getNumGroup();
}

const DataStats& Node::datastats() const {
	return stats;
}

VariantType Node::initialValue() const {
	assert(!"initialValue() was not implemented in derived class");
}

void Node::updateValue(VariantType value) {
	assert(!"updateValue() was not implemented in derived class");
}

void Node::computeScalar(std::unordered_map<Node*,VariantType> &hash) {
	assert(!"computeScalar() was not implemented in derived class");
}

void Node::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	assert(!"computeFixed() was not implemented in derived class");
}

} } // namespace map::detail
