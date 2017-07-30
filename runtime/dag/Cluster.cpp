/**
 * @file	Cluster.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Note: using the Cluster pointer in 'prev_hash' is only possible if all clusters are first created at once
 *       Think, new objects often take the address of recently deleted objects, creating false dependencies
 * Note: careful with 'prev_hash' and how it is propagated, it will not work when spliting clusters
 */

#include "Cluster.hpp"
#include "../visitor/Visitor.hpp"


namespace map { namespace detail {

int Cluster::id_count;

Cluster::Cluster()
	: id(-1)
	, task(nullptr)
	, gen_pattern(NONE_PATTERN)
	, gen_shape()

	, gen_outdated(false)
{ }

Cluster::Cluster(Pattern pattern)
	: id(-1)
	, task(nullptr)
	, gen_pattern(pattern)
	, gen_shape()
	, gen_outdated(false)
{ }

const NodeList& Cluster::nodeList() const {
	return node_list;
}

const NodeList& Cluster::inputList() const {
	return in_list;
}

const NodeList& Cluster::outputList() const {
	return out_list;
}

const ClusterList& Cluster::prevList() const {
	return prev_list;
}

const ClusterList& Cluster::nextList() const {
	return next_list;
}

const ClusterList& Cluster::backList() const {
	return back_list;
}

const ClusterList& Cluster::forwList() const {
	return forw_list;
}

NumDim Cluster::numdim() const {
	updateAttributes();
	return gen_shape.num_dim;
}

const DataSize& Cluster::datasize() const {
	updateAttributes();
	return gen_shape.data_size;
}

const BlockSize& Cluster::blocksize() const {
	updateAttributes();
	return gen_shape.block_size;
}

const NumBlock& Cluster::numblock() const {
	updateAttributes();
	return gen_shape.num_block;
}

const GroupSize& Cluster::groupsize() const {
	updateAttributes();
	return gen_shape.group_size;
}

const NumGroup& Cluster::numgroup() const {
	updateAttributes();
	return gen_shape.num_group;
}

void Cluster::addNode(Node *node) {
	if (!is_included(node,node_list)) {
		node_list.push_back(node);
		gen_pattern += node->pattern();
		gen_outdated = true;
	}
}

void Cluster::removeNode(Node *node) {
	assert(is_included(node,node_list));
	remove_value(node,this->node_list);
	gen_outdated = true;
}

void Cluster::addInputNode(Node *node) {
	if (!is_included(node,in_list)) {
		in_list.push_back(node);
		gen_outdated = true;
	}
}

void Cluster::removeInputNode(Node *node) {
	assert(is_included(node,in_list));
	remove_value(node,in_list);
	gen_outdated = true;
}

void Cluster::addOutputNode(Node *node) {
	if (!is_included(node,out_list)) {
		out_list.push_back(node);
	}
}

void Cluster::removeOutputNode(Node *node) {
	assert(is_included(node,out_list));
	remove_value(node,out_list);
}

void Cluster::addAutoNode(Node *node) {
	if (node->isInput()) {
		addInputNode(node);
		gen_pattern += INPUT;
	} else if (node->isOutput()) {
		addOutputNode(node);
		gen_pattern += OUTPUT;
	} else {
		addNode(node);
	} 
}

void Cluster::removeAutoNode(Node *node) {
	if (node->isInput()) {
		removeInputNode(node);
	} else if (node->isOutput()) {
		removeOutputNode(node);
	} else {
		removeNode(node);
	}
}

void Cluster::addPrev(Cluster *cluster, Pattern pattern) {
	assert(cluster!=this && !isNext(cluster)); // acyclic

	auto i = std::find(prev_list.begin(),prev_list.end(),cluster);
	if (i != prev_list.end()) { // Already exists
		prevPattern(i) += pattern;
	} else { // Does not exist
		prev_list.push_back(cluster);
		prev_pat.push_back(pattern);
		prev_outdated = true;
	}
}

void Cluster::removePrev(Cluster *cluster) {
	auto p = prev_pat.begin();
	for (auto it=prev_list.begin(); it!=prev_list.end(); it++, p++) {
		if (*it == cluster) {
			prev_list.erase(it);
			prev_pat.erase(p);
			prev_outdated = true;
			// Note: 'outdate' is not propagated to next clusters since it is costly
			return;
		}
	}
	assert(0);
}

bool Cluster::isPrev(const Cluster *cluster) const {
	updatePrevious();
	return (this->prev_hash.find(cluster) != this->prev_hash.end());
}

void Cluster::addNext(Cluster *cluster, Pattern pattern) {
	assert(cluster!=this && !isPrev(cluster)); // acyclic
	
	auto i = std::find(next_list.begin(),next_list.end(),cluster);
	if (i != next_list.end()) { // Already exists
		nextPattern(i) += pattern;
	} else { // Does not exist
		next_list.push_back(cluster);
		next_pat.push_back(pattern);
	}
}

void Cluster::removeNext(Cluster *cluster) {
	auto p = next_pat.begin();
	for (auto it=next_list.begin(); it!=next_list.end(); it++, p++) {
		if (*it == cluster) {
			next_list.erase(it);
			next_pat.erase(p);
			return;
		}
	}
	assert(0);	
}

bool Cluster::isNext(const Cluster *cluster) const {
	return cluster->isPrev(this);
}

void Cluster::addBack(Cluster *cluster, Pattern pattern) { //
	assert(cluster!=this);
	// assert(isPrev(cluster)); // @@ check for this cycle as a post-stage verification of fusion?
	
	auto i = std::find(back_list.begin(),back_list.end(),cluster);
	if (i == back_list.end()) {
		back_list.push_back(cluster);
	}
}

void Cluster::removeBack(Cluster *cluster) {
	remove_value(cluster,back_list);
}

void Cluster::addForw(Cluster *cluster, Pattern pattern) { //
	assert(cluster!=this);
	// assert(isNext(cluster)); // @@ check for this cycle as a post-stage verification of fusion?
	
	auto i = std::find(forw_list.begin(),forw_list.end(),cluster);
	if (i == forw_list.end()) {
		forw_list.push_back(cluster);
	}
}

void Cluster::removeForw(Cluster *cluster) {
	remove_value(cluster,forw_list);
}

void Cluster::updateAttributes() const {
	if (!gen_outdated)
		return;

	gen_shape = DataShape();

	for (auto node : full_join(in_list,node_list)) {
		auto node_shape = node->metadata().getDataShape();
		if (node_shape.encompass(gen_shape))
			gen_shape = node_shape;
		else
			assert(gen_shape.encompass(node_shape));
	}

	gen_outdated = false;
}

void Cluster::updatePrevious() const {
	if (!prev_outdated)
		return;
	prev_hash.clear();
	for (auto prev : prevList()) {
		prev->updatePrevious();
		prev_hash.insert(prev->prev_hash.begin(),prev->prev_hash.end());
		prev_hash.insert(prev);
	}
	prev_outdated = false;
}

Pattern& Cluster::prevPattern(ClusterList::const_iterator i) {
	auto dist = std::distance(prev_list.cbegin(),i);
	auto j = prev_pat.begin();
	std::advance(j,dist);
	return *j;
}

Pattern& Cluster::prevPattern(Cluster *prev) {
	auto cit = std::find(prevList().cbegin(),prevList().cend(),prev);
	assert(cit != prevList().cend());
	return prevPattern(cit);
}

Pattern& Cluster::nextPattern(ClusterList::const_iterator i) {
	auto dist = std::distance(next_list.cbegin(),i);
	auto j = next_pat.begin();
	std::advance(j,dist);
	return *j;
}

Pattern& Cluster::nextPattern(Cluster *next) {
	auto cit = std::find(nextList().cbegin(),nextList().cend(),next);
	assert(cit != nextList().cend());
	return nextPattern(cit);
}

Pattern& Cluster::pattern() {
	return gen_pattern;
}

const Pattern& Cluster::pattern() const {
	return gen_pattern;
}

std::string Cluster::shortName() const {
	return "Cluster";
}

std::string Cluster::longName() const {
	std::string str = "Cluster {";
	for (auto node : nodeList())
		str += std::to_string(node->id);
	return str + "}";
}

std::string Cluster::signature() const {
	std::string sign = "";
	for (auto &in : in_list)
		sign += in->numdim().toString() + in->datatype().toString();
	for (auto &node : node_list)
		sign += node->signature();
	for (auto &out : out_list)
		sign += out->numdim().toString() + out->datatype().toString();
	return sign;
}

} } // namespace map::detail
