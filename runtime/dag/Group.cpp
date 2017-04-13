/**
 * @file	Group.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: initilized gen_xxx_xxx to {0,0,0,0} in the constructor if Array4 is made an aggregate
 * Note: using the Group pointer in 'prev_hash' is only possible if all groups are first created at once
 *       Think, new objects often take the address of recently deleted objects, creating false dependencies
 * Note: careful with 'prev_hash' and how it is propagated, it probably won't work when spliting groups
 */

#include "Group.hpp"
#include "../visitor/Visitor.hpp"


namespace map { namespace detail {

int Group::id_count;

Group::Group()
	: id(-1)
	, task(nullptr)
	, gen_pattern(NONE_PAT)
	, gen_num_dim(NONE_NUMDIM)
	, gen_data_size()
	, gen_block_size()
	, gen_num_block()
	, gen_outdated(false)
{ }

Group::Group(Pattern pattern)
	: id(-1)
	, task(nullptr)
	, gen_pattern(pattern)
	, gen_num_dim(D0)
	, gen_data_size()
	, gen_block_size()
	, gen_num_block()
	, gen_outdated(false)
{ }

const NodeList& Group::nodeList() const {
	return node_list;
}

const NodeList& Group::inputList() const {
	return in_list;
}

const NodeList& Group::outputList() const {
	return out_list;
}

const GroupList& Group::prevList() const {
	return prev_list;
}

const GroupList& Group::nextList() const {
	return next_list;
}

const GroupList& Group::backList() const {
	return back_list;
}

const GroupList& Group::forwList() const {
	return forw_list;
}

NumDim Group::numdim() const {
	updateAttributes();
	return gen_num_dim;
}

const DataSize& Group::datasize() const {
	updateAttributes();
	return gen_data_size;
}

const BlockSize& Group::blocksize() const {
	updateAttributes();
	return gen_block_size;
}

const NumBlock& Group::numblock() const {
	updateAttributes();
	return gen_num_block;
}

void Group::addNode(Node *node) {
	if (!is_included(node,node_list)) {
		node_list.push_back(node);
		gen_pattern += node->pattern();
		gen_outdated = true;
	}
}

void Group::removeNode(Node *node) {
	assert(is_included(node,node_list));
	remove_value(node,this->node_list);
	gen_outdated = true;
}

void Group::addInputNode(Node *node) {
	if (!is_included(node,in_list)) {
		in_list.push_back(node);
		gen_outdated = true;
	}
}

void Group::removeInputNode(Node *node) {
	assert(is_included(node,in_list));
	remove_value(node,in_list);
	gen_outdated = true;
}

void Group::addOutputNode(Node *node) {
	if (!is_included(node,out_list)) {
		out_list.push_back(node);
	}
}

void Group::removeOutputNode(Node *node) {
	assert(is_included(node,out_list));
	remove_value(node,out_list);
}

void Group::addAutoNode(Node *node) {
	if (node->isInput()) {
		addInputNode(node);
		gen_pattern += FREE;
	} else if (node->isOutput()) {
		addOutputNode(node);
		gen_pattern += FREE;
	} else {
		addNode(node);
	} 
}

void Group::removeAutoNode(Node *node) {
	if (node->isInput()) {
		removeInputNode(node);
	} else if (node->isOutput()) {
		removeOutputNode(node);
	} else {
		removeNode(node);
	}
}

void Group::addPrev(Group *group, Pattern pattern) {
	assert(group!=this && !isNext(group)); // acyclic

	auto i = std::find(prev_list.begin(),prev_list.end(),group);
	if (i != prev_list.end()) { // Already exists
		prevPattern(i) += pattern;
	} else { // Does not exist
		prev_list.push_back(group);
		prev_pat.push_back(pattern);
		prev_outdated = true;
	}
}

void Group::removePrev(Group *group) {
	auto p = prev_pat.begin();
	for (auto it=prev_list.begin(); it!=prev_list.end(); it++, p++) {
		if (*it == group) {
			prev_list.erase(it);
			prev_pat.erase(p);
			prev_outdated = true;
			// Note: 'outdate' is not propagated to next groups since it is costly
			return;
		}
	}
	assert(0);
}

bool Group::isPrev(const Group *group) const {
	updatePrevious();
	return (this->prev_hash.find(group) != this->prev_hash.end());
}

void Group::addNext(Group *group, Pattern pattern) {
	assert(group!=this && !isPrev(group)); // acyclic
	
	auto i = std::find(next_list.begin(),next_list.end(),group);
	if (i != next_list.end()) { // Already exists
		nextPattern(i) += pattern;
	} else { // Does not exist
		next_list.push_back(group);
		next_pat.push_back(pattern);
	}
}

void Group::removeNext(Group *group) {
	auto p = next_pat.begin();
	for (auto it=next_list.begin(); it!=next_list.end(); it++, p++) {
		if (*it == group) {
			next_list.erase(it);
			next_pat.erase(p);
			return;
		}
	}
	assert(0);	
}

bool Group::isNext(const Group *group) const {
	return group->isPrev(this);
}

void Group::addBack(Group *group, Pattern pattern) { //
	assert(group!=this);
	assert(isPrev(group)); // cyclic
	
	auto i = std::find(back_list.begin(),back_list.end(),group);
	if (i == back_list.end()) {
		back_list.push_back(group);
	}
}

void Group::removeBack(Group *group) {
	remove_value(group,back_list);
}

void Group::addForw(Group *group, Pattern pattern) { //
	assert(group!=this);
	assert(isNext(group)); // cyclic
	
	auto i = std::find(forw_list.begin(),forw_list.end(),group);
	if (i == forw_list.end()) {
		forw_list.push_back(group);
	}
}

void Group::removeForw(Group *group) {
	remove_value(group,forw_list);
}

void Group::updateAttributes() const {
	if (!gen_outdated)
		return;
	gen_num_dim == NONE_NUMDIM;
	for (auto node : full_join(in_list,node_list)) {
		if (gen_num_dim == NONE_NUMDIM || gen_num_dim.toInt() < node->numdim().toInt()) {
			gen_num_dim = node->numdim();
			gen_data_size = node->datasize();
			gen_block_size = node->blocksize();
			gen_num_block = node->numblock();
		}
	}
	gen_outdated = false;
}

void Group::updatePrevious() const {
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

Pattern& Group::prevPattern(GroupList::const_iterator i) {
	auto dist = std::distance(prev_list.cbegin(),i);
	auto j = prev_pat.begin();
	std::advance(j,dist);
	return *j;
}

Pattern& Group::prevPattern(Group *prev) {
	auto cit = std::find(prevList().cbegin(),prevList().cend(),prev);
	assert(cit != prevList().cend());
	return prevPattern(cit);
}

Pattern& Group::nextPattern(GroupList::const_iterator i) {
	auto dist = std::distance(next_list.cbegin(),i);
	auto j = next_pat.begin();
	std::advance(j,dist);
	return *j;
}

Pattern& Group::nextPattern(Group *next) {
	auto cit = std::find(nextList().cbegin(),nextList().cend(),next);
	assert(cit != nextList().cend());
	return nextPattern(cit);
}

Pattern& Group::pattern() {
	return gen_pattern;
}

const Pattern& Group::pattern() const {
	return gen_pattern;
}

std::string Group::getName() const {
	return std::string("Group");
}

std::string Group::signature() const {
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
