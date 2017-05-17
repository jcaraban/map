/**
 * @file	DataType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: OpenCL implements bools as ints. Therefore B8 is now taking 4 bytes.
 *       In the future B8 should become B1 and implement bit-vector compression
 *       Other data types could come together, like U2 and U4.
 * TODO: Reorder the types from B1 to F64
 */

#ifndef MAP_UTIL_DATATYPE_HPP_
#define MAP_UTIL_DATATYPE_HPP_

#include <string>


namespace map { namespace detail {

/*
 * Enum
 */
enum DataTypeEnum : int { NONE_DATATYPE, F32, F64, B8, U8, U16, U32, U64, S8, S16, S32, S64, N_DATATYPE };

DataTypeEnum operator ++ (DataTypeEnum& dte);


/*
 * Class
 */
class DataType {
	DataTypeEnum type;

  public:
	DataType();
	DataType(DataTypeEnum type);
	DataTypeEnum get() const;

	bool operator==(DataType type) const;
	bool operator!=(DataType type) const;

	size_t sizeOf() const;
	std::string toString() const;
	std::string ctypeString() const;
	
	bool isUnsigned() const;
	bool isSigned() const;
	bool isFloating() const;
	bool is8() const;
	bool is16() const;
	bool is32() const;
	bool is64() const;
	
	DataType toUnsigned() const;
	DataType toSigned() const;
	DataType toFloating() const;
	DataType to8() const;
	DataType to16() const;
	DataType to32() const;
	DataType to64() const;
};

static_assert( std::is_standard_layout< DataType >::value , "DataType must be C compatible");

} } // namespace map::detail

#endif

#include "DataType.tpl"
