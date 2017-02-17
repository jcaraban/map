/**
 * @file	ThreadId.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "ThreadId.hpp"
#include "Runtime.hpp"


namespace map { namespace detail {

bool ThreadId::operator==(ThreadId &o) {
	return mch()==o.mch() && dev()==o.dev() && rnk()==o.rnk();
}

int ThreadId::proj() {
	int nd = Runtime::getConfig().num_devices;
	int nr = Runtime::getConfig().num_ranks;
	return mch()*(nd+nr) + dev()*nr + rnk();
}

} } // namespace map::detail
