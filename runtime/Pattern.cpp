/**
 * @file	Pattern.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: FLAT_L and FLAT_R are redundant, the side of the fusion does not matter in flat-fusion
 */

#include "Pattern.hpp"


namespace map { namespace detail {

PatternEnum operator + (const PatternEnum& lhs, const PatternEnum& rhs) {
	return static_cast<PatternEnum>(static_cast<int>(lhs) | static_cast<int>(rhs));
}
//PatternEnum& operator += (PatternEnum& lhs, const PatternEnum& rhs) {
//	return lhs = static_cast<PatternEnum>(static_cast<int>(lhs) | static_cast<int>(rhs));
//}
//PatternEnum operator << (const PatternEnum& lhs, int i) {
//	return static_cast<PatternEnum>(static_cast<int>(lhs) << i);
//}
PatternEnum& operator <<= (PatternEnum& lhs, int i) {
	return lhs = static_cast<PatternEnum>(static_cast<int>(lhs) << i);
}

bool Pattern::operator==(const Pattern& pattern) {
	return this->pat == pattern.pat;
}

bool Pattern::operator!=(const Pattern& pattern) {
	return this->pat != pattern.pat;
}

Pattern Pattern::operator+(const Pattern& rhs) {
	Pattern ret;
	ret.pat = this->pat + rhs.pat;
	return ret;
}

Pattern& Pattern::operator+=(const Pattern& rhs) {
	this->pat = this->pat + rhs.pat;
	return *this;
}

bool Pattern::is(Pattern pat) const {
	return (this->pat & pat.pat) == pat.pat;
}

bool Pattern::isNot(Pattern pat) const {
	return !is(pat);
}

bool canPipeFuse(const Pattern& top, const Pattern& bot) {
	#define PIPE(t,b,r) if ((top.pat & t) == t && (bot.pat & b) == b) return r;
	#define PIPE_T(t,r) if ((top.pat & t) == t) return r;
	#define PIPE_B(b,r) if ((bot.pat & b) == b) return r;

	PIPE(SPREAD,SPREAD,false)
	PIPE(SPREAD,RADIAL,false)
	PIPE(SPREAD,ZONAL,false)
	PIPE(SPREAD,FOCAL,false)
	PIPE(SPREAD,LOCAL,false)
	PIPE(SPREAD,FREE,true)
	PIPE(SPREAD,SPECIAL,false)
	PIPE(SPREAD,BARRIER,false)

	PIPE(RADIAL,SPREAD,false)
	PIPE(RADIAL,RADIAL,false) // @ RAD | RAD can be fused if 'start' is close enough, but scan of scan doesn't make sense
	PIPE(RADIAL,ZONAL,false) // @ RAD | ZONAL can be fused, skeleton not ready
	PIPE(RADIAL,FOCAL,false) // Incomplatible data dependencies
	PIPE(RADIAL,LOCAL,true)
	PIPE(RADIAL,FREE,true)
	PIPE(RADIAL,SPECIAL,false)
	PIPE(RADIAL,BARRIER,false)
	
	PIPE(ZONAL,SPREAD,false)
	PIPE(ZONAL,RADIAL,false)
	PIPE(ZONAL,ZONAL,false)
	PIPE(ZONAL,FOCAL,false)
	PIPE(ZONAL,LOCAL,false)
	PIPE(ZONAL,FREE,true)
	PIPE(ZONAL,SPECIAL,true)
	PIPE(ZONAL,BARRIER,false)

	PIPE(FOCAL,SPREAD,false)
	PIPE(FOCAL,RADIAL,false) // FOCAL | RAD can be fused, skeleton not ready
	PIPE(FOCAL,ZONAL,false) // @ FocalZonal
	PIPE(FOCAL,FOCAL,false) // FOCAL | FOCAL can be fused, skeleton not ready
	PIPE(FOCAL,LOCAL,true)
	PIPE(FOCAL,FREE,true)
	PIPE(FOCAL,SPECIAL,false)
	PIPE(FOCAL,BARRIER,false)

	PIPE(LOCAL,SPREAD,false)
	PIPE(LOCAL,RADIAL,true)
	PIPE(LOCAL,ZONAL,true)
	PIPE(LOCAL,FOCAL,false) // @ otherwise the skeleton fails if LOCAL is visited first by a non-FOCAl next-node
	PIPE(LOCAL,LOCAL,true)
	PIPE(LOCAL,FREE,true)
	PIPE(LOCAL,SPECIAL,false)
	PIPE(LOCAL,BARRIER,true)

	PIPE(SPECIAL,SPREAD,false)
	PIPE(SPECIAL,RADIAL,false)
	PIPE(SPECIAL,ZONAL,false)
	PIPE(SPECIAL,FOCAL,false)
	PIPE(SPECIAL,LOCAL,false)
	PIPE(SPECIAL,FREE,true)
	PIPE(SPECIAL,SPECIAL,false)
	PIPE(SPECIAL,BARRIER,false)

	PIPE(BARRIER,SPREAD,false)
	PIPE(BARRIER,RADIAL,false)
	PIPE(BARRIER,ZONAL,false)
	PIPE(BARRIER,FOCAL,false)
	PIPE(BARRIER,LOCAL,false)
	PIPE(BARRIER,FREE,true)
	PIPE(BARRIER,SPECIAL,false)
	PIPE(BARRIER,BARRIER,false)

	PIPE_T(FREE,true) // everything can be fused when top=FREE
	PIPE_B(FREE,true) // everything can be fused when bot=Free

	std::cerr << "Error in pipe-fusion, top: " << top << " bot: " << bot << std::endl;
	assert(0);
	return false;

	#undef PIPE
	#undef PIPE_T
	#undef PIPE_B
}

bool canFlatFuse(const Pattern& left, const Pattern& right) {
	#define FLAT(l,r,ret) if ((left.pat & l) == l && (right.pat & r) == r) return ret;
	#define FLAT_L(l,ret) if ((left.pat & l) == l) return ret;
	#define FLAT_R(r,ret) if ((right.pat & r) == r) return ret;

	FLAT(SPREAD,SPREAD,false)
	FLAT(SPREAD,RADIAL,false)
	FLAT(SPREAD,ZONAL,false)
	FLAT(SPREAD,FOCAL,false)
	FLAT(SPREAD,LOCAL,false)
	FLAT(SPREAD,FREE,true)

	FLAT(RADIAL,SPREAD,false)
	FLAT(RADIAL,RADIAL,false) // RAD + RAD can be fused if 'start' is close
	FLAT(RADIAL,ZONAL,false) // RAD + ZONAL can be fused, skeleton not ready
	FLAT(RADIAL,FOCAL,false) // RAD + FOCAL can be fused, skeleton not ready
	FLAT(RADIAL,LOCAL,true)
	FLAT(RADIAL,FREE,true)

	FLAT(ZONAL,SPREAD,false)
	FLAT(ZONAL,RADIAL,false) // ZONAL + RAD can be fused, skeleton not ready
	FLAT(ZONAL,ZONAL,true)
	FLAT(ZONAL,FOCAL,false) // @ FocalZonal
	FLAT(ZONAL,LOCAL,true)
	FLAT(ZONAL,FREE,true)

	FLAT(FOCAL,SPREAD,false) // cannot be fused within kernel, incomplatible data dependencies
	FLAT(FOCAL,RADIAL,false) // FOCAL + RAD can be fused, code not ready
	FLAT(FOCAL,ZONAL,false) // @ FocalZonal
	FLAT(FOCAL,FOCAL,true) // ...take care with different halo sizes
	FLAT(FOCAL,LOCAL,true)
	FLAT(FOCAL,FREE,true)

	FLAT_L(LOCAL,true) // everything can be fused when left=Local
	FLAT_R(LOCAL,true) // everything can be fused when right=Local

	FLAT_L(SPECIAL,true)
	FLAT_R(SPECIAL,true)

	FLAT_L(BARRIER,true)
	FLAT_R(BARRIER,true)

	FLAT_L(FREE,true) // everything can be fused when left=Free
	FLAT_R(FREE,true) // everything can be fused when right=Free

	assert(0);
	return false;

	#undef FLAT
	#undef FLAT_L
	#undef FLAT_R
}

std::ostream& operator<< (std::ostream& os, const Pattern& pat) {
	/**/ if (pat.is(BARRIER))
		os << "Barrier";
	else if (pat.is(SPECIAL))
		os << "Special";
	else if (pat.is(SPREAD))
		os << "Spreading";
	else if (pat.is(RADIAL))
		os << "Radiating";
	else if (pat.is(FOCAL+ZONAL))
		os << "FocalZonal";
	else if (pat.is(ZONAL))
		os << "Zonal";
	else if (pat.is(FOCAL))
		os << "Focal";
	else if (pat.is(LOCAL))
		os << "Local";
	else if (pat.is(FREE))
		os << "Free";
	else if (pat.is(NONE_PAT))
		os << "None";
	else
		assert(0);
	return os;
}

} } // namespace map::detail
