/**
 * @file	Write.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Write.hpp"
#include "../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

Write::Key::Key(Write *node) {
	prev = node->prev();
	path = node->file()->getFilePath();
}

bool Write::Key::operator==(const Key& k) const {
	return (prev==k.prev && path==k.path);
}

std::size_t Write::Hash::operator()(const Key& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<std::string>()(k.path);
}

// Factory

Node* Write::Factory(Node *prev, std::string file_path) {
	assert(prev != nullptr);
	assert(prev->numdim() != D0);

	// Creates an instance of File<Format>, the format is infered from the path
	SharedFile out_file = SharedFile( IFile::Factory(file_path) );
	if (out_file == nullptr) {
		assert(!"File format coudln't be infered");
	}
	// Attempts to set the data configuration
	Ferr ferr = out_file->setMetaData(prev->metadata(),OUT);
	if (ferr) {
		assert(0);
	}
	// Attemtps to open the file for writting
	ferr = out_file->open(file_path, OUT);
	if (ferr) {
		assert(!"File couldn't be opened\n");
	}
	// @ Sets statistics
	out_file->setDataStats(prev->datastats());

	// Creates and evaluates the Write node
	return new Write(prev,out_file);
}

Node* Write::clone(std::unordered_map<Node*,Node*> other_to_this) {
	return new Write(this,other_to_this);
}

// Constructors

Write::Write(Node *prev, SharedFile out_file) :
	IONode(out_file,OutputNodeFlag()), // because of virtual inheritance
	OutputNode(prev,out_file)
{ }

Write::Write(const Write *other, std::unordered_map<Node*,Node*> other_to_this)
	: IONode(other,other_to_this)
	, OutputNode() // @ InputNode(other) ?
{ }

// Methods

void Write::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string Write::getName() const {
	return "Write";
}

std::string Write::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += file()->getFilePath();
	return sign;
}

} } // namespace map::detail
