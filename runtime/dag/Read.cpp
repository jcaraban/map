/**
 * @file	Read.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Read.hpp"
#include "../Runtime.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Read::Key::Key(Read *node) {
	path = node->file()->getFilePath();
}

bool Read::Key::operator==(const Key& k) const {
	return (path==k.path);
}

std::size_t Read::Hash::operator()(const Key& k) const {
	return std::hash<std::string>()(k.path);
}

// Factory

Node* Read::Factory(std::string file_path) {
	assert(!file_path.empty());

	// Creates an instance of File<Format>
	SharedFile in_file = SharedFile( IFile::Factory(file_path) );
	if (in_file == nullptr) {
		assert(!"File format coudln't be infered\n");
	}
	// Attempts to open the file for reading
	Ferr ferr = in_file->open(file_path, IN);
	if (ferr) {
		assert(!"File couldn't be opened\n");
	}

	// Reading the file is lazy
	return new Read(in_file);
}

Node* Read::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new Read(this,other_to_this);
}

// Constructors

Read::Read(SharedFile in_file)
	: IONode(in_file,InputNodeFlag()) // because of virtual inheritance
	, InputNode(in_file)
{ }

Read::Read(const Read *other, std::unordered_map<Node*,Node*> other_to_this)
	: IONode(other,other_to_this)
	, InputNode() // @ InputNode(other) ?
{ }

// Methods

void Read::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Read::getName() const {
	return "Read";
}

std::string Read::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += file()->getNumDim().toString();
	sign += file()->getDataType().toString();
	sign += file()->getFilePath();
	return sign;
}

} } // namespace map::detail
