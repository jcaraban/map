/**
 * @file	MemOrder.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_UTIL_MEMORDER_HPP_
#define MAP_UTIL_MEMORDER_HPP_

#include <string>


namespace map { namespace detail {

/*
 * Enum
 */
enum MemOrderEnum : int { NONE_MEMORDER=0x00, BLK=0x01, ROW=0x02, COL=0x04, SFC=0x06, N_MEMORDER=0x08 };

constexpr MemOrderEnum operator+(const MemOrderEnum& lhs, const MemOrderEnum& rhs) {
	return static_cast<MemOrderEnum>( static_cast<int>(lhs) | static_cast<int>(rhs) );
}

/*
 * Class
 */
class MemOrder {
	MemOrderEnum order;

  public:
	MemOrder();
	MemOrder(MemOrderEnum order);
	MemOrderEnum get() const;

	bool operator==(MemOrder order) const;
	bool operator!=(MemOrder order) const;
	bool is(MemOrder order) const;

	std::string toString() const;
};

static_assert( std::is_standard_layout< MemOrder >::value , "MemOrder must be C compatible");

} } // namespace map::detail

#endif
