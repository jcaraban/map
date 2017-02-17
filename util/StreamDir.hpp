/**
 * @file	StreamDir.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_UTIL_STREAMDIR_HPP_
#define MAP_UTIL_STREAMDIR_HPP_

#include "../cle/cle.hpp"
#include <string>


namespace map { namespace detail {

// Enum

enum StreamDirEnum : int { NONE_STREAMDIR=0, IN=1, OUT=2, IO=3, N_STREAMDIR=4 };

// Class

class StreamDir {
	StreamDirEnum dir;

  public:
  	StreamDir();
  	StreamDir(StreamDirEnum dir);
  	StreamDirEnum get() const;

  	bool operator==(StreamDir dir) const;
  	bool operator!=(StreamDir dir) const;

	MemFlag toMemFlag() const;
	std::string toString() const;
};

} } // namespace map::detail

#endif
