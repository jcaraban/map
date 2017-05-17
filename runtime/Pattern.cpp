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

PatternEnum operator - (const PatternEnum& lhs, const PatternEnum& rhs) {
	return static_cast<PatternEnum>(static_cast<int>(lhs) & ~static_cast<int>(rhs));
}

bool Pattern::operator==(const Pattern& pattern) const {
	return this->pat == pattern.pat;
}

bool Pattern::operator!=(const Pattern& pattern) const {
	return this->pat != pattern.pat;
}

Pattern Pattern::operator+(const Pattern& rhs) {
	return Pattern(this->pat + rhs.pat);
}

Pattern& Pattern::operator+=(const Pattern& rhs) {
	this->pat = this->pat + rhs.pat;
	return *this;
}

Pattern Pattern::operator-(const Pattern& rhs) {
	return Pattern(this->pat - rhs.pat);
}

Pattern& Pattern::operator-=(const Pattern& rhs) {
	this->pat = this->pat - rhs.pat;
	return *this;
}

bool Pattern::is(Pattern pat) const {
	return (this->pat & pat.pat) == pat.pat;
}

bool Pattern::isNot(Pattern pat) const {
	return !is(pat);
}

std::string Pattern::toString() const {
	std::string str;
	if (pat == NONE_PATTERN)
		str += "None";
	if (pat == INPUT)
		str += "Input";
	if (pat == OUTPUT)
		str += "Output";
	if (is(FREE))
		str += "Free";
	if (is(LOCAL))
		str += "Local";
	if (is(FOCAL))
		str += "Focal";
	if (is(ZONAL))
		str += "Zonal";
	if (is(RADIAL))
		str += "Radial";
	if (is(SPREAD))
		str += "Spread";
	if (is(STATS))
		str += "Stats";
	if (is(GLOBAL))
		str += "Barrier";
	if (is(HEAD))
		str += "Head";
	if (is(MERGE))
		str += "Merge";
	if (is(SWITCH))
		str += "Switch";
	if (is(TAIL))
		str += "Tail";
	if (is(LOOP))
		str += "Loop";
	return str;
}

bool canPipeFuse(const Pattern& top, const Pattern& bot) {
	#define PIPE(t,b,val) if ((top.pat & t) == t && (bot.pat & b) == b) fuses &= val;
	#define PIPE_T(t,val) if ((top.pat & t) == t) fuses &= val;
	#define PIPE_B(b,val) if ((bot.pat & b) == b) fuses &= val;
	// NB: commented means true
	bool fuses = true;

	PIPE_T(FREE,true) // everything can be fused when top=FREE
	PIPE_B(FREE,true) // everything can be fused when bot=Free

	PIPE(LOCAL,FREE,true)
	PIPE(LOCAL,LOCAL,true)
	PIPE(LOCAL,FOCAL,	false) // could, not ready
	PIPE(LOCAL,ZONAL,true)
	PIPE(LOCAL,RADIAL,	false)
	PIPE(LOCAL,SPREAD,	false) // could
	PIPE(LOCAL,STATS,	false) // could
	PIPE(LOCAL,GLOBAL,true)

	PIPE(FOCAL,FREE,true)
	PIPE(FOCAL,LOCAL,true)
	PIPE(FOCAL,FOCAL,	false) // FOCAL | FOCAL can be fused, skeleton not ready
	PIPE(FOCAL,ZONAL,true) // @ FocalZonal
	PIPE(FOCAL,RADIAL,	false) // FOCAL | RAD could be fused, not interesting
	PIPE(FOCAL,SPREAD,	false)
	PIPE(FOCAL,STATS,	false)
	PIPE(FOCAL,GLOBAL,	false)
	
	PIPE(ZONAL,FREE,true)
	PIPE(ZONAL,LOCAL,	false)
	PIPE(ZONAL,FOCAL,	false)
	PIPE(ZONAL,ZONAL,	false)
	PIPE(ZONAL,RADIAL,	false)
	PIPE(ZONAL,SPREAD,	false)
	PIPE(ZONAL,STATS,true) // @ stats
	PIPE(ZONAL,GLOBAL,	false)

	PIPE(RADIAL,FREE,true)
	PIPE(RADIAL,LOCAL,	false)
	PIPE(RADIAL,FOCAL,	false) // Incomplatible data dependencies
	PIPE(RADIAL,ZONAL,	false) // RAD | ZONAL can be fused, skeleton not ready
	PIPE(RADIAL,RADIAL,	false) // RAD | RAD can be fused if 'start' is close enough, but scan of scan doesn't make sense
	PIPE(RADIAL,SPREAD,	false)
	PIPE(RADIAL,STATS,	false)
	PIPE(RADIAL,GLOBAL,	false)

	PIPE(SPREAD,FREE,true)
	PIPE(SPREAD,LOCAL,	false)
	PIPE(SPREAD,FOCAL,	false)
	PIPE(SPREAD,ZONAL,	false)
	PIPE(SPREAD,RADIAL,	false)
	PIPE(SPREAD,SPREAD,	false)
	PIPE(SPREAD,STATS,	false)
	PIPE(SPREAD,GLOBAL,	false)
	
	PIPE(STATS,FREE,true)
	PIPE(STATS,LOCAL,	false)
	PIPE(STATS,FOCAL,	false)
	PIPE(STATS,ZONAL,	false)
	PIPE(STATS,RADIAL,	false)
	PIPE(STATS,SPREAD,	false)
	PIPE(STATS,STATS,true)
	PIPE(STATS,GLOBAL,	false)

	PIPE(GLOBAL,FREE,true)
	PIPE(GLOBAL,LOCAL,	false)
	PIPE(GLOBAL,FOCAL,	false)
	PIPE(GLOBAL,ZONAL,	false)
	PIPE(GLOBAL,RADIAL,	false)
	PIPE(GLOBAL,SPREAD,	false)
	PIPE(GLOBAL,STATS,	false)
	PIPE(GLOBAL,GLOBAL,	false)

	PIPE_T(HEAD,true)
	PIPE_B(HEAD,	false)

	PIPE_T(MERGE,true)
	PIPE_B(MERGE,	false)

	PIPE_T(SWITCH,	false)
	PIPE_B(SWITCH,true)

	PIPE_T(TAIL,	false)
	PIPE_B(TAIL,true)

	PIPE_T(LOOP,true)
	PIPE_B(LOOP,true)

	return fuses;

	#undef PIPE
	#undef PIPE_T
	#undef PIPE_B
}

bool canFlatFuse(const Pattern& left, const Pattern& right) {
	#define FLAT(l,r,val) if ((left.pat & l) == l && (right.pat & r) == r) fuses &= val;
	#define FLAT_L(l,val) if ((left.pat & l) == l) fuses &= val;
	#define FLAT_R(r,val) if ((right.pat & r) == r) fuses &= val;
	bool fuses = true;

	FLAT_L(FREE,true) // everything can be fused when left=Free
	FLAT_R(FREE,true) // everything can be fused when right=Free

	FLAT_L(LOCAL,true) // everything can be fused when left=Local
	FLAT_R(LOCAL,true) // everything can be fused when right=Local

	FLAT(FOCAL,FREE,true)
	FLAT(FOCAL,LOCAL,true)
	FLAT(FOCAL,FOCAL,true) // ...take care with different halo sizes
	FLAT(FOCAL,ZONAL,true) // @ FocalZonal
	FLAT(FOCAL,RADIAL,	false) // FOCAL + RAD can be fused, code not ready
	FLAT(FOCAL,SPREAD,	false) // cannot be fused within kernel, incomplatible data dependencies

	FLAT(ZONAL,FREE,true)
	FLAT(ZONAL,LOCAL,true)
	FLAT(ZONAL,FOCAL,true) // @ FocalZonal
	FLAT(ZONAL,ZONAL,true)
	FLAT(ZONAL,RADIAL,	false) // ZONAL + RAD can be fused, skeleton not ready
	FLAT(ZONAL,SPREAD,	false)

	FLAT(RADIAL,FREE,true)
	FLAT(RADIAL,LOCAL,true)
	FLAT(RADIAL,FOCAL,	false) // RAD + FOCAL can be fused, skeleton not ready
	FLAT(RADIAL,ZONAL,	false) // RAD + ZONAL can be fused, skeleton not ready
	FLAT(RADIAL,RADIAL,	false) // RAD + RAD can be fused if 'start' is close
	FLAT(RADIAL,SPREAD,	false)

	FLAT(SPREAD,FREE,true)
	FLAT(SPREAD,LOCAL,	false)
	FLAT(SPREAD,FOCAL,	false)
	FLAT(SPREAD,ZONAL,	false)
	FLAT(SPREAD,RADIAL,	false)
	FLAT(SPREAD,SPREAD,	false)

	FLAT_L(STATS,true)
	FLAT_R(STATS,true)

	FLAT_L(GLOBAL,true)
	FLAT_R(GLOBAL,true)

	FLAT_L(HEAD,true)
	FLAT_R(HEAD,true)

	FLAT_L(MERGE,true)
	FLAT_R(MERGE,true)

	FLAT_L(SWITCH,true)
	FLAT_R(SWITCH,true)

	FLAT_L(TAIL,true)
	FLAT_R(TAIL,true)

	FLAT_L(LOOP,true)
	FLAT_R(LOOP,true)

	return fuses;

	#undef FLAT
	#undef FLAT_L
	#undef FLAT_R
}

std::ostream& operator<< (std::ostream& os, const Pattern& pat) {
	return os << pat.toString();
}

} } // namespace map::detail
