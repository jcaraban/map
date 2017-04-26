/**
 * @file	RadialScan.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "RadialScan.hpp"
#include "../visitor/Visitor.hpp"
#include "../../util/Direction.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

RadialScan::Content::Content(RadialScan *node) {
	prev = node->prev();
	type = node->type;
	start = node->start;
}

bool RadialScan::Content::operator==(const Content& k) const {
	return (prev==k.prev && type==k.type && all(start==k.start));
}

std::size_t RadialScan::Hash::operator()(const Content& k) const {
	size_t hash = std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
	for (int i=0; i<k.start.size(); i++)
		hash ^= std::hash<int>()(k.start[i]);
	return hash;
}

// Factory

Node* RadialScan::Factory(Node *arg, ReductionType type, Coord start) {
	assert(arg != nullptr);
	assert(arg->numdim() != D0);

	DataSize ds = arg->datasize();
	DataType dt = arg->datatype();
	MemOrder mo = arg->memorder();
	BlockSize bs = arg->blocksize();
	MetaData meta(ds,dt,mo,bs);

	return new RadialScan(meta,arg,type,start);
}

Node* RadialScan::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new RadialScan(this,other_to_this);
}

// Constructors

RadialScan::RadialScan(const MetaData &meta, Node *prev, ReductionType type, Coord start)
	: Node(meta)
{
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);
	
	this->type = type;
	this->start = start;

	this->in_spatial_reach = Mask(numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true);
}

RadialScan::RadialScan(const RadialScan *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
	this->start = other->start;
}

// Methods

void RadialScan::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string RadialScan::getName() const {
	return "RadialScan";
}

std::string RadialScan::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	sign += to_string(start);
	return sign;
}

Node* RadialScan::prev() const {
	return prev_list[0];
}

// Spatial

const Mask& RadialScan::inputReach(Coord coord) const {
	return center; // @@

	auto startb = this->start / blocksize();
	auto dif = abs(startb - coord);
	int x = coord[0] - startb[0];
	int y = coord[1] - startb[1];

	if (all(dif == 0)) // Center
	{
		// { 0, 0, 0 }
		// { 0, 1, 0 }
		// { 0, 0, 0 }
		return center;
	}
	else if (sum(dif) == 1)  // Center +- 1 in only 1 dir
	{
		if (x==0 && y < 0) { // North
			// { 0, 0, 0 }
			// { 0, 1, 0 }
			// { 0, 1, 0 }
			return north;
		}
		else if (x > 1 && y==0) {
			// { 1, 1, 0 }
			// { 1, 1, 0 }
			// { 1, 1, 0 }
			return east;	
		}
		else if (x==0 && y > 0) {
			// { 1, 1, 1 }
			// { 1, 1, 1 }
			// { 0, 0, 0 }
			return south;
		}
		else if (x > 1 && y==0) {
			// { 1, 1, 0 }
			// { 1, 1, 0 }
			// { 1, 1, 0 }
			return east;	
		}
		else {
			assert(0);
		}
	}
	else if (any(dif == 0)) // Compass
	{
		;
	}
	else if (std::abs(dif[0]-dif[1]) <= 1) // Diagonal (inner and upper too)
	{
		;
	}
	else // Sector
	{
		;
	}
}

// Compute

void RadialScan::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	hash[{node,coord}] = ValFix();
	// @ if max summary value of the block is lower than the accumulated, then fix it!
}

} } // namespace map::detail
