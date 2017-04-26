/**
 * @file	Summary.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Summary.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Summary::Content::Content(Summary *node) {
	prev = node->prev();
}

bool Summary::Content::operator==(const Content& k) const {
	return (prev==k.prev);
}

std::size_t Summary::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Summary::Factory(Node *prev, Node *min, Node *max, Node *mean, Node *std) {
	assert(prev != nullptr);
	if (min != nullptr) {
		auto node = dynamic_cast<BlockSummary*>(min);
		assert(node != nullptr);
		assert(node->type == MIN);
		assert(all(min->datasize() == prev->datasize()));
		assert(all(min->blocksize() == prev->blocksize()));
		assert(min->datatype() == prev->datatype());
		assert(min->numdim() == prev->numdim());
	}
	if (max != nullptr) {
		auto node = dynamic_cast<BlockSummary*>(max);
		assert(node != nullptr);
		assert(node->type == MAX);
		assert(all(max->datasize() == prev->datasize()));
		assert(all(max->blocksize() == prev->blocksize()));
		assert(max->datatype() == prev->datatype());
		assert(max->numdim() == prev->numdim());
	}
	if (mean != nullptr) {
		// @
		assert(all(mean->datasize() == prev->datasize()));
		assert(all(mean->blocksize() == prev->blocksize()));
		assert(mean->datatype() == prev->datatype());
		assert(mean->numdim() == prev->numdim());
	}
	if (std != nullptr) {
		// @
		assert(all(std->datasize() == prev->datasize()));
		assert(all(std->blocksize() == prev->blocksize()));
		assert(std->datatype() == prev->datatype());
		assert(std->numdim() == prev->numdim());
	}

	DataSize ds = prev->datasize();
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = prev->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new Summary(meta,prev,min,max,mean,std);
}

Node* Summary::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Summary(this,other_to_this);
}

// Constructors

Summary::Summary(const MetaData &meta, Node *prev, Node *min, Node *max, Node *mean, Node *std)
	: Node(meta)
{
	this->addPrev(prev);
	prev->addNext(this);
	if (min != nullptr) {
		this->addPrev(min);
		min->addNext(this);
	}
	if (max != nullptr) {
		this->addPrev(max);
		max->addNext(this);
	}
	if (mean != nullptr) {
		this->addPrev(mean);
		mean->addNext(this);
	}
	if (std != nullptr) {
		this->addPrev(std);
		std->addNext(this);
	}

	// Prepares statistics structure
	const int num = prod(numblock());
	this->stats.active = true;
	this->stats.maxb.resize(num);
	this->stats.meanb.resize(num);
	this->stats.minb.resize(num);
	this->stats.stdb.resize(num);

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

Summary::Summary(const Summary *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{ }

// Methods

void Summary::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Summary::getName() const {
	return "Summary";
}

std::string Summary::signature() const {
	std::string sign = "";
	sign += classSignature();
	for (auto node : prevList()) {
		if (node != nullptr) {
			sign += prev()->numdim().toString();
			sign += prev()->datatype().toString();
		}
	}
	return sign;
}

Node* Summary::prev() const {
	return prev_list[0];
}

Node* Summary::min() const {
	return prev_list.size() > 1 ? prev_list[1] : nullptr;
}

Node* Summary::max() const {
	return prev_list.size() > 2 ? prev_list[2] : nullptr;
}

Node* Summary::mean() const {
	return prev_list.size() > 3 ? prev_list[3] : nullptr;
}

Node* Summary::std() const {
	return prev_list.size() > 4 ? prev_list[4] : nullptr;
}

// Compute

void Summary::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	// @ something else ?
	hash[{this,coord}] = hash.find({prev(),coord})->second;
}

} } // namespace map::detail
