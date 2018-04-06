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

#include <bitset>


namespace map { namespace detail {

/*
 * Enumerate storing the possible different patterns
 */
enum PatternEnum {
	NONE_PATTERN,
	INPUT, OUTPUT, FREE, LOCAL, FOCAL, ZONAL, RADIAL, SPREAD,
	STATS, GLOBAL, HEAD, TAIL, MERGE, SWITCH, LOOP,
	N_PATTERN
};

static_assert( N_PATTERN <= 8*sizeof(uint64_t), "PatternEnum cannot surpasses the max. supported size" );


/*
 * Pattern class
 * Represent multiple Map Algebra patterns / classes, and can change at running time
 */
struct Pattern {
	typedef std::bitset<N_PATTERN> bitset;
	bitset bits; //!< Holds one bit for every pattern

	Pattern() : bits() { } //!< Zero initialization of bits [0,0,0,...,0]
	Pattern(PatternEnum pat) { if(pat) bits.set(pat); } //!< Maps pattern to bits
  private:
	Pattern(const bitset& bits) : bits(bits) { }
  public:

	bool operator==(const Pattern& pat) const;
	bool operator!=(const Pattern& pat) const;
	/*
	 * Is() differs with == in that the latter compares all the patterns
	 * Therefore is(Local), is(Focal) and ==Local+Focal all return true. 
	 */
	bool is(Pattern pat) const;
	bool isNot(Pattern pat) const;
	/*
	 * Modifier operators
	 */
	Pattern operator+(const Pattern& rhs);
	Pattern& operator+=(const Pattern& rhs);
	Pattern operator-(const Pattern& rhs);
	Pattern& operator-=(const Pattern& rhs);

	bool operator<(const Pattern& rhs) const;
	bool operator>(const Pattern& rhs) const;

	size_t hash() const;

	//
	std::string toString() const;
	
	//friend bool canPipeFuse(const Pattern& top, const Pattern& bot);
	//friend bool canFlatFuse(const Pattern& left, const Pattern& right);
	//friend std::ostream& operator<< (std::ostream& os, const Pattern& pat);
};

/*
 * prints the pattern
 */
std::string toString(const PatternEnum& pat);
std::ostream& operator<< (std::ostream& os, const PatternEnum& pat);
std::ostream& operator<< (std::ostream& os, const Pattern& pat);

/*
 * returns true when 'top' and 'bot' can be pipe-fused following the top-bot direcction
 */
bool canPipeFuse(const Pattern& top, const Pattern& bot);

/*
 * returns true when 'left' and 'right' can be flat-fused
 */
bool canFlatFuse(const Pattern& left, const Pattern& right);

} } // namespace map::detail

#endif
