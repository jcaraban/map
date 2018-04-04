/**
 * @file	NumDim.tpl
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_UTIL_NUMDIM_TPL_
#define MAP_UTIL_NUMDIM_TPL_


namespace map { namespace detail {

template <NumDimEnum D> struct NumDim2Int { static_assert(true,"Invalid NumDim template parameter"); };
template <> struct  NumDim2Int< D0 > { static const int value = 0; };
template <> struct  NumDim2Int< D1 > { static const int value = 1; };
template <> struct  NumDim2Int< D2 > { static const int value = 2; };
template <> struct  NumDim2Int< D3 > { static const int value = 3; };
template <> struct  NumDim2Int< D0+TIME > { static const int value = 1; };
template <> struct  NumDim2Int< D1+TIME > { static const int value = 2; };
template <> struct  NumDim2Int< D2+TIME > { static const int value = 3; };
template <> struct  NumDim2Int< D3+TIME > { static const int value = 4; };

// Int2NumDim is impossible, the function is not injective

} } // namespace map::detail

#endif
