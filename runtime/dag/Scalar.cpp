/**
 * @file	Scalar.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Scalar.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Scalar::Content::Content(Scalar *node) {
	prev = node->prev();
}

bool Scalar::Content::operator==(const Content& k) const {
	return (prev==k.prev);
}

std::size_t Scalar::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev);
}

// Factory

Node* Scalar::Factory(Node *prev) {
	assert(prev != nullptr);
	assert(prev->numdim() == D0);
	
	// Creates scalar file
	File<scalar> *sca_file = new File<scalar>(prev->metadata());
	if (prev->pattern().is(ZONAL)) {
		auto *red = dynamic_cast<ZonalReduc*>(prev);
		sca_file->setReductionType(red->type);
	} else {
		sca_file->setReductionType(NONE_REDUCTION);
	}

	// Creates and evaluates Scalar node
	return new Scalar(prev,SharedFile(sca_file));
}

Node* Scalar::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Scalar(this,other_to_this);
}

// Constructors

Scalar::Scalar(Node *prev, SharedFile sca_file) :
	IONode(sca_file,OutputNodeFlag()), // because of virtual inheritance
	OutputNode(prev,sca_file)
{ }

Scalar::Scalar(const Scalar *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: IONode(other,other_to_this)
	, OutputNode() // @ InputNode(other) ?
{ }

// Methods

void Scalar::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Scalar::getName() const {
	return "Scalar";
}

std::string Scalar::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += file()->getFilePath();
	return sign;
}

VariantType Scalar::value() {
	auto *sca_file = dynamic_cast<File<scalar>*>(file().get());
	assert(sca_file != nullptr);
	return sca_file->value();
}

// Compute

void Scalar::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	hash[{this,coord}] = hash.find({prev(),coord})->second;
}

} } // namespace map::detail
