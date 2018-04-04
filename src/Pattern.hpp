/**
 * @file	Pattern.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: a Histogram Pattern is necessary for the kind of computation that needs a lookup table in shared memory.
 * TODO: UpSample and DownSample might also need a patter (which allow accessing more than the inmediate nbh blocks)
 *
 * TODO: create variadic AND OR functions that checks if pattern is several patterns at once
 */

#ifndef MAP_RUNTIME_PATTERN_HPP_
#define MAP_RUNTIME_PATTERN_HPP_

#include "util/Array.hpp"


namespace map { namespace detail {

/*
 * Enumerate storing the possible different patterns
 */
enum PatternEnum { NONE_PATTERN=0x00, INPUT=0x01, OUTPUT=0x02, FREE=0x04, LOCAL=0x08, FOCAL=0x10, ZONAL=0x20, RADIAL=0x40, SPREAD=0x80,
					STATS=0x100, GLOBAL=0x200, HEAD=0x400, TAIL=0x800, MERGE=0x1000, SWITCH=0x2000, LOOP=0x4000, N_PATTERN=0x8000 };

PatternEnum operator + (const PatternEnum& lhs, const PatternEnum& rhs);
PatternEnum operator - (const PatternEnum& lhs, const PatternEnum& rhs);

/*
 * Pattern class
 * Can represent any pattern (or mix of patterns) and can change during running time
 */
struct Pattern {
	PatternEnum pat; //!< Internal state of the object which reflects the actual pattern
	Pattern() : pat(NONE_PATTERN) { }
	Pattern(PatternEnum pat) : pat(pat) { }

	bool operator==(const Pattern& pattern) const;
	bool operator!=(const Pattern& pattern) const;

	/*
	 * Modifier operators
	 */
	Pattern operator+(const Pattern& rhs);
	Pattern& operator+=(const Pattern& rhs);
	Pattern operator-(const Pattern& rhs);
	Pattern& operator-=(const Pattern& rhs);

	/*
	 * Is() differs with == in that Pattern might be more things,like Local+Focal.
	 * In such case is(Local), is(Focal) and ==Local+Focal all return true. 
	 */
	bool is(Pattern pat) const;
	bool isNot(Pattern pat) const;

	//
	std::string toString() const;
	
	//friend bool canPipeFuse(const Pattern& top, const Pattern& bot);
	//friend bool canFlatFuse(const Pattern& left, const Pattern& right);
	friend std::ostream& operator<< (std::ostream& os, const Pattern& pat);
};

/*
 * returns true when 'top' and 'bot' can be pipe-fused following the top-bot direcction
 */
bool canPipeFuse(const Pattern& top, const Pattern& bot);

/*
 * returns true when 'left' and 'right' can be flat-fused
 */
bool canFlatFuse(const Pattern& left, const Pattern& right);

/*
 * prints the pattern
 */
std::ostream& operator<< (std::ostream& os, const Pattern& pat);

} } // namespace map::detail

#endif
