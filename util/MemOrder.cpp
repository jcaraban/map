/**
 * @file	MemOrder.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "MemOrder.hpp"
#include <cassert>


namespace map { namespace detail {

MemOrder::MemOrder()
	: order(NONE_MEMORDER)
{ }

MemOrder::MemOrder(MemOrderEnum order) {
	assert(order >= NONE_MEMORDER && order < N_MEMORDER);
	this->order = order;
}

MemOrderEnum MemOrder::get() const {
	return order;
}

bool MemOrder::operator==(MemOrder order) const {
	return this->order == order.get();
}

bool MemOrder::operator!=(MemOrder order) const {
	return this->order != order.get();
}

bool MemOrder::is(MemOrder order) const {
	return (this->order & order.get()) == order.get();
}

std::string MemOrder::toString() const {
	switch (order) {
		case NONE_MEMORDER : return std::string("NONE_MEMORDER");
		case ROW : return std::string("ROW");
		case COL : return std::string("COL");
		case SFC : return std::string("SFC");
		case ROW+BLK : return std::string("ROW+BLK");
		case COL+BLK : return std::string("COL+BLK");
		default : assert(0);
	}
}

std::ostream& operator<< (std::ostream& os, const MemOrder& order) {
	return os << order.toString();
}

} } // namespace map::detail
