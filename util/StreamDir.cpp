/**
 * @file	StreamDir.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "StreamDir.hpp"


namespace map { namespace detail {

StreamDir::StreamDir()
	: dir(NONE_STREAMDIR)
{ }

StreamDir::StreamDir(StreamDirEnum dir) {
	assert(dir >= NONE_STREAMDIR && dir < N_STREAMDIR);
	this->dir = dir;
}

StreamDirEnum StreamDir::get() const {
	return dir;
}

bool StreamDir::operator==(StreamDir dir) const {
	return this->dir == dir.get();
}

bool StreamDir::operator!=(StreamDir dir) const {
	return this->dir != dir.get();
}

MemFlag StreamDir::toMemFlag() const {
	switch (dir) {
		case IN  : return MEM_READ_ONLY;
		case OUT : return MEM_WRITE_ONLY;
		case IO  : return MEM_READ_WRITE;
		default  : assert(0);
	}
}

std::string StreamDir::toString() const {
	switch (dir) {
		case NONE_STREAMDIR : return std::string("NONE_STREAMDIR"); 
		case IN  : return std::string("IO");
		case OUT : return std::string("OUT");
		case IO  : return std::string("IO");
		default  : assert(0);
	}
	return 0;
}

} } // namespace map::detail
