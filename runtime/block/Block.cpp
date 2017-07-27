/**
 * @file	Block.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: reduce constructors to just one ?
 */

#include "Block.hpp"
#include "../dag/Node.hpp"


namespace map { namespace detail {

Block::Block()
	: key()
	, dependencies(DEPEND_UNKNOWN)
	, ready(false)
	, dirty(false)
	, used(0)
	, order(-1)
	, mtx()
{ }

Block::Block(Key key, int dep)
	: key(key)
	, dependencies(dep)
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
	assert(dependencies > 0);
	dependencies--;
}

bool Block::discardable() const {
	return dependencies == DEPEND_ZERO; // == 0
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
