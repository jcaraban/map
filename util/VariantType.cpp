/**
 * @file	VariantType.cpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "VariantType.hpp"
#include <functional>
#include <limits>
#include <cmath>
#include <cassert>


namespace map { namespace detail {

VariantType::VariantType() : type() { var.f64 = 0.0; }
VariantType::VariantType(VariantUnion var, DataType type) : var(var), type(type) { }
VariantType::VariantType( Ctype<F32> val ) : type(F32) { ref<F32>() = val; }
VariantType::VariantType( Ctype<F64> val ) : type(F64) { ref<F64>() = val; }
VariantType::VariantType( Ctype<B8 > val ) : type(B8 ) { ref<B8 >() = val; }
VariantType::VariantType( Ctype<U8 > val ) : type(U8 ) { ref<U8 >() = val; }
VariantType::VariantType( Ctype<U16> val ) : type(U16) { ref<U16>() = val; }
VariantType::VariantType( Ctype<U32> val ) : type(U32) { ref<U32>() = val; }
VariantType::VariantType( Ctype<U64> val ) : type(U64) { ref<U64>() = val; }
VariantType::VariantType( Ctype<S8 > val ) : type(S8 ) { ref<S8 >() = val; }
VariantType::VariantType( Ctype<S16> val ) : type(S16) { ref<S16>() = val; }
VariantType::VariantType( Ctype<S32> val ) : type(S32) { ref<S32>() = val; }
VariantType::VariantType( Ctype<S64> val ) : type(S64) { ref<S64>() = val; }
VariantType::VariantType( Ctype<F32> val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<F64> val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<B8 > val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<U8 > val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<U16> val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<U32> val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<U64> val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<S8 > val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<S16> val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<S32> val, DataType dt) { set(val,dt); }
VariantType::VariantType( Ctype<S64> val, DataType dt) { set(val,dt); }

DataType VariantType::datatype() const {
	return type;
}

bool VariantType::isEqual(VariantType other) const {
	assert(this->type.get() != NONE_DATATYPE);
	assert(other.type.get() != NONE_DATATYPE);
	if (this->type != other.type)
		return false;
	switch (type.get()) {
		case F32 :
		{
			Ctype<F32> a = this->get<F32>();
			Ctype<F32> b = other.get<F32>();
			Ctype<F32> eps = std::numeric_limits<Ctype<F32>>::epsilon() * 2;
			return (std::abs(a - b) <= eps * std::max(std::abs(a),std::abs(b)));
		}
		case F64 :
		{
			Ctype<F64> a = this->get<F64>();
			Ctype<F64> b = other.get<F64>();
			Ctype<F64> eps = std::numeric_limits<Ctype<F64>>::epsilon() * 2;
			return (std::abs(a - b) <= eps * std::max(std::abs(a),std::abs(b)));
		}
		case B8  : return get<B8 >() == other.get<B8 >();
		case U8  : return get<U8 >() == other.get<U8 >();
		case U16 : return get<U16>() == other.get<U16>();
		case U32 : return get<U32>() == other.get<U32>();
		case U64 : return get<U64>() == other.get<U64>();
		case S8  : return get<S8 >() == other.get<S8 >();
		case S16 : return get<S16>() == other.get<S16>();
		case S32 : return get<S32>() == other.get<S32>();
		case S64 : return get<S64>() == other.get<S64>();
		default: assert(0);
	}
}

bool VariantType::operator==(VariantType other) const {
	return isEqual(other);
}

bool VariantType::operator!=(VariantType other) const {
	return ! isEqual(other);
}

VariantType::operator bool() const {
	return convert(B8).get<B8>();
}

size_t VariantType::hash() const {
	size_t hash = std::hash<int>()(type.get());
	switch (type.get()) {
		case F32 : return hash ^ std::hash< Ctype<F32> >()(get<F32>());
		case F64 : return hash ^ std::hash< Ctype<F64> >()(get<F64>());
		case B8  : return hash ^ std::hash< Ctype<B8 > >()(get<B8 >());
		case U8  : return hash ^ std::hash< Ctype<U8 > >()(get<U8 >());
		case U16 : return hash ^ std::hash< Ctype<U16> >()(get<U16>());
		case U32 : return hash ^ std::hash< Ctype<U32> >()(get<U32>());
		case U64 : return hash ^ std::hash< Ctype<U64> >()(get<U64>());
		case S8  : return hash ^ std::hash< Ctype<S8 > >()(get<S8 >());
		case S16 : return hash ^ std::hash< Ctype<S16> >()(get<S16>());
		case S32 : return hash ^ std::hash< Ctype<S32> >()(get<S32>());
		case S64 : return hash ^ std::hash< Ctype<S64> >()(get<S64>());
		default: assert(0);
	}
}

std::string VariantType::toString() const {
	switch (type.get()) {
		case F32 : return std::to_string(get<F32>());
		case F64 : return std::to_string(get<F64>());
		case B8  : return std::to_string(get<B8 >());
		case U8  : return std::to_string(get<U8 >());
		case U16 : return std::to_string(get<U16>());
		case U32 : return std::to_string(get<U32>());
		case U64 : return std::to_string(get<U64>());
		case S8  : return std::to_string(get<S8 >());
		case S16 : return std::to_string(get<S16>());
		case S32 : return std::to_string(get<S32>());
		case S64 : return std::to_string(get<S64>());
		default: assert(0);
	}
}

bool VariantType::isNone() const {
	return type == NONE_DATATYPE;
}

bool VariantType::isZero() const {
	switch (type.get()) {
		case F32 : return (get<F32>() == 0);
		case F64 : return (get<F64>() == 0);
		case B8  : return (get<B8 >() == 0);
		case U8  : return (get<U8 >() == 0);
		case U16 : return (get<U16>() == 0);
		case U32 : return (get<U32>() == 0);
		case U64 : return (get<U64>() == 0);
		case S8  : return (get<S8 >() == 0);
		case S16 : return (get<S16>() == 0);
		case S32 : return (get<S32>() == 0);
		case S64 : return (get<S64>() == 0);
		default: assert(0);
	}
}

bool VariantType::isOne() const {
	assert(0);
}

VariantUnion& VariantType::ref() {
	return var;
}

VariantUnion VariantType::get() const {
	return var;
}

void VariantType::set(VariantUnion var, DataType dt) {
	//this->var = var;
	//return *this;
	assert(0); // not sure about this function
}

VariantType VariantType::convert(DataType dt) const {
	switch (type.get()) {
		case F32 : return VariantType(get<F32>(),dt);
		case F64 : return VariantType(get<F64>(),dt);
		case B8  : return VariantType(get<B8 >(),dt);
		case U8  : return VariantType(get<U8 >(),dt);
		case U16 : return VariantType(get<U16>(),dt);
		case U32 : return VariantType(get<U32>(),dt);
		case U64 : return VariantType(get<U64>(),dt);
		case S8  : return VariantType(get<S8 >(),dt);
		case S16 : return VariantType(get<S16>(),dt);
		case S32 : return VariantType(get<S32>(),dt);
		case S64 : return VariantType(get<S64>(),dt);
		default: assert(0);
	}
}

void VariantType::fill(void *mem, size_t num) const {
	switch (type.get()) {
		case F32 : std::fill_n((Ctype<F32>*)mem,num,get<F32>()); break;
		case F64 : std::fill_n((Ctype<F64>*)mem,num,get<F64>()); break;
		case B8  : std::fill_n((Ctype<B8 >*)mem,num,get<B8 >()); break;
		case U8  : std::fill_n((Ctype<U8 >*)mem,num,get<U8 >()); break;
		case U16 : std::fill_n((Ctype<U16>*)mem,num,get<U16>()); break;
		case U32 : std::fill_n((Ctype<U32>*)mem,num,get<U32>()); break;
		case U64 : std::fill_n((Ctype<U64>*)mem,num,get<U64>()); break;
		case S8  : std::fill_n((Ctype<S8 >*)mem,num,get<S8 >()); break;
		case S16 : std::fill_n((Ctype<S16>*)mem,num,get<S16>()); break;
		case S32 : std::fill_n((Ctype<S32>*)mem,num,get<S32>()); break;
		case S64 : std::fill_n((Ctype<S64>*)mem,num,get<S64>()); break;
		default: assert(0);
	}
}

std::ostream& operator<<(std::ostream &strm, const VariantType &var) {
	return strm << var.toString();
}

} } // namespace map::detail
