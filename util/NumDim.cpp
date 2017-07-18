/**
 * @file	NumDim.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "NumDim.hpp"
#include <cassert>
 

namespace map { namespace detail {

NumDim::NumDim()
	: dim(NONE_NUMDIM)
{ }

NumDim::NumDim(NumDimEnum dim) {
	assert(dim >= NONE_NUMDIM && dim < N_NUMDIM);
	this->dim = dim;
}

NumDimEnum NumDim::get() const {
	return dim;
}

bool NumDim::operator==(NumDim dim) const {
	return this->dim == dim.get();
}

bool NumDim::operator!=(NumDim dim) const {
	return this->dim != dim.get();
}

bool NumDim::is(NumDim dim) const {
	return (this->dim & dim.get()) == dim.get();
}

int NumDim::toInt() const {
	switch (dim) {
		case D0 : return 0;
		case D1 : return 1;
		case D2 : return 2;
		case D3 : return 3;
		case D0+TIME : return 1;
		case D1+TIME : return 2;
		case D2+TIME : return 3;
		case D3+TIME : return 4;
		default : assert(0);
	}
}

std::string NumDim::toString() const {
	switch (dim) {
		case NONE_NUMDIM : return std::string("NONE_NUMDIM");
		case D0 : return std::string("D0");
		case D1 : return std::string("D1");
		case D2 : return std::string("D2");
		case D3 : return std::string("D3");
		case D0+TIME : return std::string("TIME");
		case D1+TIME : return std::string("D1+TIME");
		case D2+TIME : return std::string("D2+TIME");
		case D3+TIME : return std::string("D3+TIME");
		default : assert(0);
	}
}

Coord NumDim::unitVec() const {
	switch (dim) {
		case D0 : return Coord(0);
		case D1 : return Coord{1};
		case D2 : return Coord{1,1};
		case D3 : return Coord{1,1,1};
		default : assert(0);
	}
}

NumDim DataSize2NumDim(const DataSize &ds) {
	switch (ds.size()) {
		case 0: return D0;
		case 1: return D1;
		case 2: return D2;
		case 3: return D3;
		default: assert(0);
	}
}

} } // namespace map::detail
