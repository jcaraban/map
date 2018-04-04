/**
 * @file	Block.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Block.hpp"
#include "block/Block0.hpp"
#include "block/Block1.hpp"
#include "block/BlockN.hpp"
#include "../intermediate/Node.hpp"


namespace map { namespace detail {

Block* Block::Factory(Key key, HoldType hold, int dep, cl_mem block_page, cl_mem group_page, int max_size) {
	Block *blk = nullptr;

	if (hold == HOLD_0) // Null block that holds '0' values
	{
		blk = new Block0(key,dep);
	}
	else if (hold == HOLD_1) // Block holds '1' value
	{
		blk = new Block1(key,dep,block_page);
	}
	//else if (hold == HOLD_2) // Block holds 'groups per block' values
	//{
	//	blk = new Block2(key,dep,group_page);
	//}
	else if (hold == HOLD_N) // Normal case, block holds 'N' values
	{
		blk = new BlockN(key,dep,max_size);
	}
	else {
		assert(0);
	}

	return blk;
}

Block::Block()
	: key()
	, depend(DEPEND_UNKNOWN)
	, ready(false)
	, dirty(false)
	, used(0)
	, order(-1)
	, mtx()
{ }

Block::Block(Key key, int dep)
	: key(key)
	, depend(dep)
	, ready(false)
	, dirty(false)
	, used(0)
	, order(-1)
	, mtx()
{ }

Block::~Block() { }

Node* Block::node() const {
	return key.node;
}

Coord Block::coord() const {
	return key.coord;
}

StreamDir Block::streamdir() const {
	return node()->streamdir();
}

DataType Block::datatype() const {
	return node()->datatype();
}

NumDim Block::numdim() const {
	return node()->numdim();
}

MemOrder Block::memorder() const {
	return node()->memorder();
}

int Block::size() const {
	return -1; // invalid value
}

//

Berr Block::preload() {
	// nothing to do in the general case
}

Berr Block::evict() {
	// nothing to do in the general case
}

Berr Block::load() {
	// nothing to do in the general case
}

Berr Block::store() {
	// nothing to do in the general case
}

Berr Block::init() {
	// nothing to do in the general case
}

Berr Block::reduce() {
	// nothing to do in the general case
}

bool Block::needEntry() const {
	return false;
}

bool Block::giveEntry() const {
	return false;
}

Entry* Block::getEntry() const {
	return nullptr;
}

void Block::setEntry(Entry *entry) {
	assert(0);
}

void Block::unsetEntry() {
	assert(0);
}

std::shared_ptr<IFile> Block::getFile() const {
	return nullptr; // invalid value
}

void Block::setFile(std::shared_ptr<IFile> file) {
	// nothing to do in the general case
}

void* Block::getHostMem() const {
	assert(0);
}

void Block::setHostMem(void *host) {
	// nothing to do in the general case
}

void Block::unsetHostMem() {
	// nothing to do in the general case
}

void* Block::getDevMem() {
	return nullptr; // @
}

CellStats Block::getStats() const {
	return CellStats(); // @
}

void Block::setStats(CellStats sta) {
	assert(0);
}

VariantType Block::getValue() const {
	return VariantType(); // @
}

void Block::setValue(VariantType val) {
	assert(0);
}

bool Block::isFixed() const {
	assert(0);
}

void Block::fixValue(VariantType val) {
	assert(0);
}

void Block::setForward() {
	assert(0);
}

void Block::unsetForward() {
	assert(0);
}

bool Block::isForward() const {
	return false; // @
}

void Block::forwardEntry(Block *out) {
	assert(0);
}

void Block::notify() {
	assert(depend > 0);
	depend--;
}

bool Block::discardable() const {
	return depend == DEPEND_ZERO; // == 0
}

void Block::setReady() {
	assert(not ready);
	ready = true;
}

void Block::unsetReady() {
	assert(ready);
	ready = false;
}

bool Block::isReady() const {
	return ready;
}

void Block::setDirty() {
	assert(not dirty);
	dirty = true;
	if (getEntry() != nullptr)
		getEntry()->setDirty();
}

void Block::unsetDirty() {
	assert(dirty);
	dirty = false;
	if (getEntry() != nullptr)
		getEntry()->unsetDirty();	
}

bool Block::isDirty() const {
	if (getEntry() != nullptr)
		assert(dirty == getEntry()->dirty);
	return dirty;
}

void Block::setUsed() {
	used++;
	if (getEntry() != nullptr)
		getEntry()->setUsed();
}

void Block::unsetUsed() {
	assert(used > 0);
	used--;
	if (getEntry() != nullptr)
		getEntry()->unsetUsed();
}

bool Block::isUsed() const {
	if (getEntry() != nullptr)
		assert(used == getEntry()->used);
	return used > 0;
}

} } // namespace map::detail
