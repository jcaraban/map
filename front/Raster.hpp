/**
 * @file	Raster.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Raster is the front-end C++ class.
 * Raster is visible to the user (map::Raster)
 */

#ifndef MAP_FRONT_RASTER_HPP_
#define MAP_FRONT_RASTER_HPP_

#include "../util/util.hpp"
#include <initializer_list>


namespace map { namespace detail {

typedef int Rerr;
class Raster; // forward declaration
struct Node; // forward declaration

/*
 * @class RasterLhsAcc
 * Sugar-Syntax class for left access assignment
 */
class RasterLhsAcc {
	Raster *orig;
	Coord coord;
  public:
	RasterLhsAcc(Raster *orig, Coord coord);
	Raster operator=(Raster other);
	Raster operator+=(Raster other);
	operator Raster() const;
};

/*
 * @class Raster
 * Front-end class to be used by the user.
 * A Raster object is a wrapper of a Node, which represents data resulting from elemental operations
 */
class Raster
{
  public: // @
	/*************
	   Variables
	 *************/

	Node *node; //!< Pointer to the node in the DAG that represents this data

	explicit Raster(Node *node); //!< Private constructor used together with elemental functions

	void incr();
	void decr();

  public:
	/*****************************************
	   Construction, Destruction, Copy, Move
	 *****************************************/

	/*
	 * Default Constructor
	 */
	Raster();
	
	/*
	 * Destructor
	 */
	~Raster();
	
	/*
	 * Copy Constructor
	 */
	Raster(const Raster &other);
	
	/*
	 * Copy Assignment
	 */
	Raster operator=(Raster other);
	
	/***************************
	   Constructors, Operators
	 ***************************/

	/*
	 * Scalar constructor
	 * shall it be explicit? would that help?
	 */
	Raster(VariantType val);

	/*
	 * D0 Assignment
	 */
	Raster operator=(VariantType value);

	/*
	 * Access operator
	 * Returns an auxiliar RasterLhsAcc object that will:
	 * a) become a plain Access node if invoked on the right side of expressions
	 * b) become a LeftAccess node together with the op=, if invoked on the left side
	 */
	RasterLhsAcc operator[](const Coord &coord);

	/*
	 * Neighbor Gather operator
	 */
	Raster operator()(const Coord &coord);

	/*
	 * Neighbor Spread operator
	 */
	Raster operator()(Raster dir, ReductionType red);	

	/***********
	   Getters
	 ***********/
	
	//StreamDir streamdir() const;
	DataType datatype() const;
	NumDim numdim() const;
	MemOrder memorder() const;
	DataSize datasize() const;
	BlockSize blocksize() const;
	//NumBlock numblock() const;
	BlockSize groupsize() const;
};

/*****************
   Other Methods
 *****************/

void setupDevices(std::string plat_name, DeviceType dev, std::string dev_name);
void eval(std::initializer_list<Raster> list);

/*********************
   Elemental Methods
 *********************/

Rerr info(Raster data);
Raster stats(Raster data, Raster min=Raster(), Raster max=Raster(), Raster mean=Raster(), Raster std=Raster());
Raster barrier(Raster data);

VariantType value(Raster data);

Raster read(const std::string &file_path);
Rerr write(Raster data, const std::string &file_path);

Raster zeros(DataSize ds=DataSize(0), DataType dt=F32, MemOrder mo=ROW+BLK, BlockSize bs=BlockSize(0), GroupSize gs=GroupSize(0));
Raster zeros_like(Raster data, DataType type=NONE_DATATYPE, MemOrder mem_order=NONE_MEMORDER);
Raster ones(DataSize data_size=DataSize(0), DataType data_type=F32, MemOrder mem_order=ROW+BLK, BlockSize block_size=BlockSize(0));
Raster ones_like(Raster data, DataType type=NONE_DATATYPE, MemOrder mem_order=NONE_MEMORDER);
Raster full_like(VariantType var, Raster data, DataType type=NONE_DATATYPE, MemOrder mem_order=NONE_MEMORDER);

Raster rand(VariantType seed, DataSize ds=DataSize(0), DataType dt=F32, MemOrder mo=ROW+BLK, BlockSize bs=BlockSize(0), GroupSize gs=GroupSize(0));
Raster rand(Raster seed, DataType type=NONE_DATATYPE, MemOrder mem_order=NONE_MEMORDER);

Raster astype(Raster data, DataType data_type);
Raster index(Raster data, NumDim dim=D1);

Raster con(Raster cond, Raster lhs, Raster rhs);
Raster con(VariantType cond, Raster lhs, Raster rhs);
Raster con(Raster cond, VariantType lhs, Raster rhs);
Raster con(Raster cond, Raster lhs, VariantType rhs);
Raster con(Raster cond, VariantType lhs, VariantType rhs);

#define DECLARE_UNARY_OP(op) \
	Raster operator op (Raster data);

	DECLARE_UNARY_OP(+)
	DECLARE_UNARY_OP(-)
	DECLARE_UNARY_OP(!)
	DECLARE_UNARY_OP(~)
#undef DECLARE_UNARY_OP
	
#define DECLARE_UNARY_FUNC(name) \
	Raster name(Raster data);

	DECLARE_UNARY_FUNC(sin)
	DECLARE_UNARY_FUNC(cos)
	DECLARE_UNARY_FUNC(tan)
	DECLARE_UNARY_FUNC(asin)
	DECLARE_UNARY_FUNC(acos)
	DECLARE_UNARY_FUNC(atan)
	DECLARE_UNARY_FUNC(sinh)
	DECLARE_UNARY_FUNC(cosh)
	DECLARE_UNARY_FUNC(tanh)
	DECLARE_UNARY_FUNC(asinh)
	DECLARE_UNARY_FUNC(acosh)
	DECLARE_UNARY_FUNC(atanh)
	DECLARE_UNARY_FUNC(exp)
	DECLARE_UNARY_FUNC(exp2)
	DECLARE_UNARY_FUNC(exp10)
	DECLARE_UNARY_FUNC(log)
	DECLARE_UNARY_FUNC(log2)
	DECLARE_UNARY_FUNC(log10)
	DECLARE_UNARY_FUNC(sqrt)
	DECLARE_UNARY_FUNC(cbrt)
	DECLARE_UNARY_FUNC(abs)
	DECLARE_UNARY_FUNC(ceil)
	DECLARE_UNARY_FUNC(floor)
	DECLARE_UNARY_FUNC(trunc)
	DECLARE_UNARY_FUNC(round)
#undef DECLARE_UNARY_FUNC


#define DECLARE_BINARY_OP(op) \
	Raster operator op (Raster lhs, Raster rhs); \
	Raster operator op (Raster lhs, VariantType rhs); \
	Raster operator op (VariantType lhs, Raster rhs);

	DECLARE_BINARY_OP(+)
	DECLARE_BINARY_OP(-)
	DECLARE_BINARY_OP(*)
	DECLARE_BINARY_OP(/)
	DECLARE_BINARY_OP(%)
	DECLARE_BINARY_OP(==)
	DECLARE_BINARY_OP(!=)
	DECLARE_BINARY_OP(<)
	DECLARE_BINARY_OP(>)
	DECLARE_BINARY_OP(<=)
	DECLARE_BINARY_OP(>=)
	DECLARE_BINARY_OP(&&)
	DECLARE_BINARY_OP(||)
	DECLARE_BINARY_OP(&)
	DECLARE_BINARY_OP(|)
	DECLARE_BINARY_OP(^)
	DECLARE_BINARY_OP(<<)
	DECLARE_BINARY_OP(>>)
#undef DECLARE_BINARY_OP


#define DECLARE_BINARY_FUNC(name) \
	Raster name(Raster lhs, Raster rhs); \
	Raster name(Raster lhs, VariantType rhs); \
	Raster name(VariantType lhs, Raster rhs);

	DECLARE_BINARY_FUNC(max)
	DECLARE_BINARY_FUNC(min)
	DECLARE_BINARY_FUNC(atan2)
	DECLARE_BINARY_FUNC(pow)
	DECLARE_BINARY_FUNC(hypot)
	DECLARE_BINARY_FUNC(fmod)
#undef DECLARE_BINARY_FUNC


#define DECLARE_DIVERSITY(name) \
	Raster name(Raster d1, Raster d2, Raster d3, Raster d4);

	DECLARE_DIVERSITY(variety)
	DECLARE_DIVERSITY(majority)
	DECLARE_DIVERSITY(minority)
	DECLARE_DIVERSITY(mean)
#undef DECLARE_DIVERSITY

Raster convolve(Raster data, const Mask &mask);


 #define DECLARE_FOCAL(name) \
	Raster focal##name(Raster data, const Mask &mask);

	DECLARE_FOCAL(Sum)
	DECLARE_FOCAL(Prod)
	DECLARE_FOCAL(Max)
	DECLARE_FOCAL(Min)
#undef DECLARE_FOCAL


#define DECLARE_PERCENT(name) \
	Raster name(Raster data, const Mask &mask);

	DECLARE_PERCENT(percentage)
	DECLARE_PERCENT(percentile)
#undef DECLARE_PERCENT


#define DECLARE_REDUCTION(name) \
	Raster zonal##name(Raster data);

	DECLARE_REDUCTION(Sum)
	DECLARE_REDUCTION(Prod)
	DECLARE_REDUCTION(Max)
	DECLARE_REDUCTION(Min)
#undef DECLARE_REDUCTION

/********************
   Compound Methods
 ********************/
	
Raster pick(Raster data, std::vector<VariantType> vec);
Raster distance(Raster data, Coord coord);
Raster square(Raster data);
//Raster slope(Raster data, float z_factor=1, float dist=1);
//Raster aspect(Raster data, float dist=1);
//Raster flowDirection(Raster data);
//Raster flowDir(Raster data);
Raster viewshed(Raster data, Coord obs, float obs_h);
Raster flowAccumulation(Raster data, Raster dir);
Raster flowAccuGather(Raster data, Raster dir, int steps);
Raster flowAccuSpread(Raster data, Raster dir, int steps);
Raster pitFill(Raster data, Raster dir, int steps);

} } // namespace map::detail

#endif
