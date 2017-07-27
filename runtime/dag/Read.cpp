/**
 * @file	Read.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Read.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Read::Content::Content(Read *node) {
	path = node->getFile()->getFilePath();
}

bool Read::Content::operator==(const Content& k) const {
	return (path==k.path);
}

std::size_t Read::Hash::operator()(const Content& k) const {
	return std::hash<std::string>()(k.path);
}

// Factory

Node* Read::Factory(std::string file_path) {
	assert(!file_path.empty());

	// Creates an instance of File<Format> according to the file extension
	SharedFile in_file = SharedFile( IFile::Factory(file_path) );
	if (in_file == nullptr) {
		assert(!"File format coudln't be infered\n");
	}
	// Attempts to open the file for reading with the retrieved File<Format>
	Ferr ferr = in_file->open(file_path, IN);
	if (ferr) {
		assert(!"File couldn't be opened\n");
	}

	// Only the metadata was read, the actual data is loaded while computing
	return new Read(in_file);
}

Node* Read::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new Read(this,other_to_this);
}

// Constructors

Read::Read(SharedFile in_file)
	: IONode(in_file,InputNodeFlag()) // because of virtual inheritance
	, InputNode(in_file)
{ }

Read::Read(const Read *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: IONode(other,other_to_this)
	, InputNode() // @ InputNode(other) ?
{ }

// Methods

void Read::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Read::shortName() const {
	return "Read";
}

std::string Read::longName() const {
	std::string str = "Read {" + getFile()->getFilePath() + "}";
	return str;
}

std::string Read::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += getFile()->getNumDim().toString();
	sign += getFile()->getDataType().toString();
	sign += getFile()->getFilePath();
	return sign;
}

} } // namespace map::detail
