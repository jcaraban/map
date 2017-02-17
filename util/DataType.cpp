/**
 * @file	DataType.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "DataType.hpp"
#include <cassert>


namespace map { namespace detail {

DataType::DataType()
	: type(NONE_DATATYPE)
{ }

DataType::DataType(DataTypeEnum type) {
	assert(type >= NONE_DATATYPE && type < N_DATATYPE);
	this->type = type;
}

DataTypeEnum DataType::get() const {
	return type;
}

bool DataType::operator==(DataType type) const {
	return this->get() == type.get();
}

bool DataType::operator!=(DataType type) const {
	return this->get() != type.get();
}

size_t DataType::sizeOf() const {
	switch (type) {
		case F32 : return sizeof( Ctype<F32> );
		case F64 : return sizeof( Ctype<F64> );
		case B8  : return sizeof( Ctype<B8 > );
		case U8  : return sizeof( Ctype<U8 > );
		case U16 : return sizeof( Ctype<U16> );
		case U32 : return sizeof( Ctype<U32> );
		case U64 : return sizeof( Ctype<U64> );
		case S8  : return sizeof( Ctype<S8 > );
		case S16 : return sizeof( Ctype<S16> );
		case S32 : return sizeof( Ctype<S32> );
		case S64 : return sizeof( Ctype<S64> );
		default  : assert(0);
	}
	return 0;
}

std::string DataType::toString() const {
	switch (type) {
		case NONE_DATATYPE : return std::string("NONE_DATATYPE"); 
		case F32 : return std::string("F32");
		case F64 : return std::string("F64");
		case B8  : return std::string("B8");
		case U8  : return std::string("U8");
		case U16 : return std::string("U16");
		case U32 : return std::string("U32");
		case U64 : return std::string("U64");
		case S8  : return std::string("S8");
		case S16 : return std::string("S16");
		case S32 : return std::string("S32");
		case S64 : return std::string("S64");
		default  : assert(0);
	}
	return 0;
}

std::string DataType::ctypeString() const {
	switch (type) {
		case F32 : return std::string("float");
		case F64 : return std::string("double");
		case B8  : return std::string("bool");
		case U8  : return std::string("uchar");
		case U16 : return std::string("ushort");
		case U32 : return std::string("uint");
		case U64 : return std::string("ulong");
		case S8  : return std::string("char");
		case S16 : return std::string("short");
		case S32 : return std::string("int");
		case S64 : return std::string("long");
		default  : assert(0);
	}
	return 0;
}

bool DataType::isUnsigned() const {
	switch (type) {
		case F32 : return 0;
		case F64 : return 0;
		case B8  : return 0;
		case U8  : return true;
		case U16 : return true;
		case U32 : return true;
		case U64 : return true;
		case S8  : return 0;
		case S16 : return 0;
		case S32 : return 0;
		case S64 : return 0;
		default  : assert(0);
	}
	return 0;
}

bool DataType::isSigned() const {
	switch (type) {
		case F32 : return 0;
		case F64 : return 0;
		case B8  : return 0;
		case U8  : return 0;
		case U16 : return 0;
		case U32 : return 0;
		case U64 : return 0;
		case S8  : return true;
		case S16 : return true;
		case S32 : return true;
		case S64 : return true;
		default  : assert(0);
	}
	return 0;
}

bool DataType::isFloating() const {
	switch (type) {
		case F32 : return true;
		case F64 : return true;
		case B8  : return 0;
		case U8  : return 0;
		case U16 : return 0;
		case U32 : return 0;
		case U64 : return 0;
		case S8  : return 0;
		case S16 : return 0;
		case S32 : return 0;
		case S64 : return 0;
		default  : assert(0);
	}
	return 0;
}

bool DataType::is8() const {
	switch (type) {
		case F32 : return 0;
		case F64 : return 0;
		case B8  : return true;
		case U8  : return true;
		case U16 : return 0;
		case U32 : return 0;
		case U64 : return 0;
		case S8  : return true;
		case S16 : return 0;
		case S32 : return 0;
		case S64 : return 0;
		default  : assert(0);
	}
	return 0;
}

bool DataType::is16() const {
	switch (type) {
		case F32 : return 0;
		case F64 : return 0;
		case B8  : return 0;
		case U8  : return 0;
		case U16 : return true;
		case U32 : return 0;
		case U64 : return 0;
		case S8  : return 0;
		case S16 : return true;
		case S32 : return 0;
		case S64 : return 0;
		default  : assert(0);
	}
	return 0;
}

bool DataType::is32() const {
	switch (type) {
		case F32 : return true;
		case F64 : return 0;
		case B8  : return 0;
		case U8  : return 0;
		case U16 : return 0;
		case U32 : return true;
		case U64 : return 0;
		case S8  : return 0;
		case S16 : return 0;
		case S32 : return true;
		case S64 : return 0;
		default  : assert(0);
	}
	return 0;
}

bool DataType::is64() const {
	switch (type) {
		case F32 : return 0;
		case F64 : return true;
		case B8  : return 0;
		case U8  : return 0;
		case U16 : return 0;
		case U32 : return 0;
		case U64 : return true;
		case S8  : return 0;
		case S16 : return 0;
		case S32 : return 0;
		case S64 : return true;
		default  : assert(0);
	}
	return 0;
}

DataType DataType::toUnsigned() const {
	switch (type) {
		case F32 : return U32;
		case F64 : return U64;
		case B8  : assert(0);
		case U8  : return U8;
		case U16 : return U16;
		case U32 : return U32;
		case U64 : return U64;
		case S8  : return U8;
		case S16 : return U16;
		case S32 : return U32;
		case S64 : return U64;
		default  : assert(0);
	}
}

DataType DataType::toSigned() const {
	switch (type) {
		case F32 : return S32;
		case F64 : return S64;
		case B8  : assert(0);
		case U8  : return S8;
		case U16 : return S16;
		case U32 : return S32;
		case U64 : return S64;
		case S8  : return S8;
		case S16 : return S16;
		case S32 : return S32;
		case S64 : return S64;
		default  : assert(0);
	}
}

DataType DataType::toFloating() const {
	switch (type) {
		/*
		case F32 : return F32;
		case F64 : return F64;
		case B8  : return F8;
		case U8  : return F8;
		case U16 : return F16;
		case U32 : return F32;
		case U64 : return F64;
		case S8  : return F8;
		case S16 : return F16;
		case S32 : return F32;
		case S64 : return F64;
		*/
		default  : assert(0);
	}
}

DataType DataType::to8() const {
	switch (type) {
		/*
		case F32 : assert(0);
		case F64 : assert(0);
		case B8  : assert(0);
		case U8  : return U8;
		case U16 : return U8;
		case U32 : return U8;
		case U64 : return U8;
		case S8  : return S8;
		case S16 : return S8;
		case S32 : return S8;
		case S64 : return S8;
		*/
		default  : assert(0);
	}
}

DataType DataType::to16() const {
	switch (type) {
		/*
		case F32 : return F16;
		case F64 : return F16;
		case B8  : assert(0);
		case U8  : return U16;
		case U16 : return U16;
		case U32 : return U16;
		case U64 : return U16;
		case S8  : return S16;
		case S16 : return S16;
		case S32 : return S16;
		case S64 : return S16;
		*/
		default  : assert(0);
	}
}

DataType DataType::to32() const {
	switch (type) {
		case F32 : return F32;
		case F64 : return F32;
		case B8  : assert(0);
		case U8  : return U32;
		case U16 : return U32;
		case U32 : return U32;
		case U64 : return U32;
		case S8  : return S32;
		case S16 : return S32;
		case S32 : return S32;
		case S64 : return S32;
		default  : assert(0);
	}
}

DataType DataType::to64() const {
	switch (type) {
		case F32 : return F64;
		case F64 : return F64;
		case B8  : assert(0);
		case U8  : return U64;
		case U16 : return U64;
		case U32 : return U64;
		case U64 : return U64;
		case S8  : return S64;
		case S16 : return S64;
		case S32 : return S64;
		case S64 : return S64;
		default  : assert(0);
	}
}

} } // namespace map::detail
