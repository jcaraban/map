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

Scalar::Key::Key(Scalar *node) {
	prev = node->prev();
}

bool Scalar::Key::operator==(const Key& k) const {
	return (prev==k.prev);
}

std::size_t Scalar::Hash::operator()(const Key& k) const {
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

Node* Scalar::clone(NodeList new_prev_list) {
	return new Scalar(this,new_prev_list);
}

// Constructors

Scalar::Scalar(Node *prev, SharedFile sca_file) :
	IONode(sca_file,OutputNodeFlag()), // because of virtual inheritance
	OutputNode(prev,sca_file)
{ }

Scalar::Scalar(const Scalar *other, NodeList new_prev_list)
	: IONode(other,new_prev_list)
	, OutputNode() // @@ InputNode(other) ?
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
	auto *sca_file = dynamic_cast<File<scalar>*>(file());
	assert(sca_file != nullptr);
	return sca_file->value();
}

} } // namespace map::detail
