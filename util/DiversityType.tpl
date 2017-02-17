/**
 * @file	DiversityType.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_UTIL_DIVERSITYTYPE_TPL_
#define MAP_UTIL_DIVERSITYTYPE_TPL_

#include <cassert>


namespace map { namespace detail {

template <DiversityEnum D>
VariantType DiversityType::apply1(std::vector<VariantType> list) {
	switch (list[0].datatype().get()) {
		case F32: return apply2<D,F32>(list);
		case F64: return apply2<D,F64>(list);
		case B8 : return apply2<D,B8 >(list);
		case U8 : return apply2<D,U8 >(list);
		case U16: return apply2<D,U16>(list);
		case U32: return apply2<D,U32>(list);
		case U64: return apply2<D,U64>(list);
		case S8 : return apply2<D,S8 >(list);
		case S16: return apply2<D,S16>(list);
		case S32: return apply2<D,S32>(list);
		case S64: return apply2<D,S64>(list);
		default: assert(0);
	}
}

namespace { // anonymous namespace
	template <typename T>
	void addElem(T e, std::vector<T> elemSA, std::vector<int> countSA, int &num) {
		for (int i=0; i<num; i++) {
			if (e == elemSA[i]) {
				countSA[i]++;
				return;
			}
		}
		elemSA[num] = e;
		countSA[num] = 1;
		num++;
	}

	template <typename T>
	T majorElem(std::vector<T> elemSA, std::vector<int> countSA, int num) {
		int major = 0;
		for (int i=1; i<num; i++)
			if (countSA[i] > countSA[major])
				major = i;
		return elemSA[major];
	}

	template <typename T>
	T minorElem(std::vector<T> elemSA, std::vector<int> countSA, int num) {
		int minor = 0;
		for (int i=1; i<num; i++)
			if (countSA[i] < countSA[minor])
				minor = i;
		return elemSA[minor];
	}

	template <typename T>
	T meanElem(std::vector<T> elemSA, std::vector<int> countSA, int num) {
		T mean = 0;
		for (int i=0; i<num; i++)
			mean += elemSA[i] * countSA[i];
		return mean / num;
	}
}

/*
 * Auxiliar Operator class, required for template-partial-specialization
 */
template <DiversityEnum D, DataTypeEnum T>
struct DiversityOperator {
	static_assert(true,"Not valid template parameter");
};

#define DEFINE_DIVERSITY_OPERATOR(D,ret,expr) \
	template <DataTypeEnum T> \
	struct DiversityOperator<D,T> { \
		Ctype<ret> operator()(std::vector<Ctype<T>> list) { \
			std::vector<Ctype<T>> elemSA; \
			std::vector<int> countSA; \
			int num = 0; \
			for (auto e : list) \
				addElem(static_cast<Ctype<T>>(e),elemSA,countSA,num); \
			return expr; \
		} \
	};

DEFINE_DIVERSITY_OPERATOR(VARI, U8, num )
DEFINE_DIVERSITY_OPERATOR(MAJO, T, majorElem(elemSA,countSA,num) )
DEFINE_DIVERSITY_OPERATOR(MINO, T, minorElem(elemSA,countSA,num) )
DEFINE_DIVERSITY_OPERATOR(MEAN, T, meanElem(elemSA,countSA,num) )
#undef DEFINE_DIVERSITY_OPERATOR

template <DiversityEnum D, DataTypeEnum T>
VariantType DiversityType::apply2(std::vector<VariantType> list) {
	std::vector<Ctype<T>> ctype_list;
	for (int i=0; i<list.size(); i++)
		ctype_list[i] = list[i].get<T>();
	return VariantType( DiversityOperator<D,T>()(ctype_list) );
}

} } // namespace map::detail

#endif
