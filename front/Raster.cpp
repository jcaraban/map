/**
 * @file	Raster.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "Raster.hpp"
#include "bindings.hpp"
#include <iostream>
#include <cassert>
#include <string>


namespace map { namespace detail {

// Free Methods

void setupDevices(std::string plat_name, DeviceType dev, std::string dev_name) {
	ma_setupDevices(plat_name.data(),dev,dev_name.data());
}

void eval(std::initializer_list<Raster> list) {
	std::vector<Node*> vec;
	for (auto raster : list)
		vec.push_back(raster.node);
	return ma_eval(vec.data(),vec.size());
}

/***************************
   RasterLhsAcc: left Access
 ***************************/

RasterLhsAcc::RasterLhsAcc(Raster *orig, Coord coord)
	: orig(orig)
	, coord(coord)
{ }

Raster RasterLhsAcc::operator=(Raster other) {
	assert(other.numdim() == D0); // Only single, no slice assignment 
	assert(orig->datatype() == other.datatype());

	// lhs[coord] = rhs --> lhs = LhsAccess( lhs, rhs, coord)
	auto node = ma_lhsAccess(orig->node,other.node,coord);

	orig->decr();
	orig->node = node; // updates original Raster object
	orig->incr();

	return Raster(node); // @ shall return *orig instead?
}

Raster RasterLhsAcc::operator+=(Raster other) {
	assert(other.numdim() == D0); // Only single, no slice assignment 

	// lhs[coord] += rhs --> lhs = LhsAccess( lhs, lhs[coord] + rhs, coord)
	auto access = ma_access(orig->node,coord);
	auto binary = ma_binary(access,other.node,ADD);
	auto node = ma_lhsAccess(orig->node,binary,coord);

	orig->decr();
	orig->node = node; // updates original Raster object
	orig->incr();

	return Raster(node);
}

RasterLhsAcc::operator Raster() const {
	// The Access operator was RHS after all
	return Raster( ma_access(orig->node,coord) );
}

/*****************************
   Constructors, Destructors
 *****************************/

void Raster::incr() {
	ma_increaseRef(node);
}

void Raster::decr() {
	ma_decreaseRef(node);
}

Raster::Raster()
	: node(nullptr)
{ }

Raster::Raster(Node *node)
	: node(node)
{
	incr();
}

Raster::~Raster() {
	decr();
}

Raster::Raster(const Raster &other)
	: node(other.node)
{
	incr();
}

Raster Raster::operator=(Raster other) {
	decr();
	node = other.node;
	incr();
	return *this;
}

/**********************
   Other Constructors
 **********************/

Raster::Raster(VariantType val) {
	node = ma_constant(val,{},val.datatype().get(),ROW,{});
	incr();
}


Raster Raster::operator=(VariantType val) {
	decr();
	node = ma_constant(val,{},val.datatype().get(),ROW,{});
	incr();
	return *this;
}

RasterLhsAcc Raster::operator[](const Coord &coord) {
	assert(all(in_range(coord,datasize())));
	return RasterLhsAcc(this,coord);
}

Raster Raster::operator()(const Coord &coord) {
	assert(all(coord > Coord{-16,-16} && coord < Coord{16,16}));
	return Raster( ma_neighbor(node,coord) );
}

Raster Raster::operator()(Raster dir, ReductionType type) {
	return Raster( ma_spreadNeighbor(node,dir.node,type.get()) );
}

/***********
   Getters
 ***********/

//StreamDir Raster::streamdir() const {
//	return ma_streamdir(node);
//}

DataType Raster::datatype() const {
	return ma_datatype(node);
}

NumDim Raster::numdim() const {
	return ma_numdim(node);
}

MemOrder Raster::memorder() const {
	return ma_memorder(node);
}

DataSize Raster::datasize() const {
	return ma_datasize(node);
}

BlockSize Raster::blocksize() const {
	return ma_blocksize(node);
}

//NumBlock Raster::numblock() const {
//	return ma_numblock(node);
//}

/*********************
   Elemental Methods
 *********************/

Rerr info(Raster data) {
	if (!data.node) {
		assert(!"Raster info not available");
	}

	std::cout << "************* INFO *************" << std::endl;
	std::cout << "Type: " << data.datatype().toString() << std::endl;
	std::cout << "NumDim: " << data.numdim().toString() << std::endl;
	std::cout << "DataSize: " << data.datasize() << std::endl;
	std::cout << "MemOrder: " << data.memorder().toString() << std::endl;
	if (data.memorder().is(BLK))
		std::cout << "BlockSize: " << data.blocksize() << std::endl;
	std::cout << "*******************************" << std::endl;

	return 0;
}

Raster stats(Raster data) {
	return Raster( ma_stats(data.node) );
}

Raster barrier(Raster data) {
	return Raster( ma_barrier(data.node) );
}

VariantType value(Raster data) {
	return ma_value(data.node);
}

Raster read(const std::string &file_path) {
	return Raster( ma_read(file_path.data()) );
}

Rerr write(Raster data, const std::string &file_path) {
	return ma_write(data.node,file_path.data());
}

Raster zeros(DataSize data_size, DataType data_type, MemOrder mem_order, BlockSize block_size) {
	VariantType var(0,data_type);
	return Raster( ma_constant(var,data_size,data_type.get(),mem_order.get(),block_size) );
}

Raster zeros_like(Raster data, DataType type, MemOrder order) {
	DataSize ds = data.datasize();
	DataType dt = (type != NONE_DATATYPE) ? type : data.datatype();
	MemOrder mo = (order != NONE_MEMORDER) ? order : data.memorder();
	BlockSize bs = data.blocksize();
	VariantType var(0,dt);
	return Raster( ma_constant(var,ds,dt.get(),mo.get(),bs) );
}

Raster ones(DataSize data_size, DataType data_type, MemOrder mem_order, BlockSize block_size) {
	VariantType var(1,data_type);
	return Raster( ma_constant(var,data_size,data_type.get(),mem_order.get(),block_size) );
}

Raster ones_like(Raster data, DataType type, MemOrder order) {
	DataSize ds = data.datasize();
	DataType dt = (type != NONE_DATATYPE) ? type : data.datatype();
	MemOrder mo = (order != NONE_MEMORDER) ? order : data.memorder();
	BlockSize bs = data.blocksize();
	VariantType var(1,dt);
	return Raster( ma_constant(var,ds,dt.get(),mo.get(),bs) );
}

Raster full(VariantType var, DataSize data_size, DataType data_type, MemOrder mem_order, BlockSize block_size) {
	return Raster( ma_constant(var,data_size,data_type.get(),mem_order.get(),block_size) );
}

Raster full_like(VariantType var, Raster data, DataType type, MemOrder order) {
	DataSize ds = data.datasize();
	DataType dt = (type != NONE_DATATYPE) ? type : var.datatype();
	MemOrder mo = (order != NONE_MEMORDER) ? order : data.memorder();
	BlockSize bs = data.blocksize();
	return Raster( ma_constant(var,ds,dt.get(),mo.get(),bs) );
}

Raster rand(VariantType seed, DataSize data_size, DataType data_type, MemOrder mem_order, BlockSize block_size) {
	seed = seed.convert(data_type);
	Raster cnst = full(seed,data_size,data_type,mem_order,block_size);
	return Raster( ma_rand(cnst.node,data_type.get(),mem_order.get()) );
}

Raster rand(Raster seed, DataType data_type, MemOrder mem_order) {
	return Raster( ma_rand(seed.node,data_type.get(),mem_order.get()) );
}

Raster astype(Raster data, DataType new_type) {
	return Raster( ma_cast(data.node,new_type.get()) );
}

Raster index(Raster data, NumDim dim) {
	DataSize ds = data.datasize();
	MemOrder mo = data.memorder();
	BlockSize bs = data.blocksize();
	return Raster( ma_index(ds,dim.get(),mo.get(),bs) );
}

Raster con(Raster cond, Raster lhs, Raster rhs) {
	return Raster( ma_conditional(cond.node,lhs.node,rhs.node) );
}

Raster con(VariantType cond, Raster lhs, Raster rhs) {
	Raster cnst = full_like(cond,lhs);
	return Raster( ma_conditional(cnst.node,lhs.node,rhs.node) );	
}

Raster con(Raster cond, VariantType lhs, Raster rhs) {
	Raster cnst = full_like(lhs,rhs);
	return Raster( ma_conditional(cond.node,cnst.node,rhs.node) );
}

Raster con(Raster cond, Raster lhs, VariantType rhs) {
	Raster cnst = full_like(rhs,cond);
	return Raster( ma_conditional(cond.node,lhs.node,cnst.node) );
}

Raster con(Raster cond, VariantType lhs, VariantType rhs) {
	assert(lhs.datatype() == rhs.datatype());
	Raster lcnst = full_like(lhs,cond);
	Raster rcnst = full_like(rhs,cond);
	return Raster( ma_conditional(cond.node,lcnst.node,rcnst.node) );
}

Raster convolve(Raster data, const Mask &mask) {
	VariantType *ptr = const_cast<VariantType*>(&mask.mask[0]);
	int size = mask.mask.size();
	DataSize ds = mask.datasize();
	return Raster( ma_convolution(data.node,ptr,size,ds) );
}

#define DEFINE_UNARY_OP(op,type) \
	Raster operator op (Raster data) { \
		return Raster( ma_unary(data.node,type) ); \
	} 

	DEFINE_UNARY_OP( + , POS  )
	DEFINE_UNARY_OP( - , NEG  )
	DEFINE_UNARY_OP( ! , NOT  )
	DEFINE_UNARY_OP( ~ , bNOT )
#undef DEFINE_UNARY_OP

#define DEFINE_UNARY_FUNC(name,type) \
	Raster name(Raster data) { \
		return Raster( ma_unary(data.node,type) ); \
	}

	DEFINE_UNARY_FUNC( sin   , SIN   )
	DEFINE_UNARY_FUNC( cos   , COS   )
	DEFINE_UNARY_FUNC( tan   , TAN   )
	DEFINE_UNARY_FUNC( asin  , ASIN  )
	DEFINE_UNARY_FUNC( acos  , ACOS  )
	DEFINE_UNARY_FUNC( atan  , ATAN  )
	DEFINE_UNARY_FUNC( sinh  , SINH  )
	DEFINE_UNARY_FUNC( cosh  , COSH  )
	DEFINE_UNARY_FUNC( tanh  , TANH  )
	DEFINE_UNARY_FUNC( asinh , ASINH )
	DEFINE_UNARY_FUNC( acosh , ACOSH )
	DEFINE_UNARY_FUNC( atanh , ATANH )
	DEFINE_UNARY_FUNC( exp   , EXP   )
	DEFINE_UNARY_FUNC( exp2  , EXP2  )
	DEFINE_UNARY_FUNC( exp10 , EXP10 )
	DEFINE_UNARY_FUNC( log   , LOG   )
	DEFINE_UNARY_FUNC( log2  , LOG2  )
	DEFINE_UNARY_FUNC( log10 , LOG10 )
	DEFINE_UNARY_FUNC( sqrt  , SQRT  )
	DEFINE_UNARY_FUNC( cbrt  , CBRT  )
	DEFINE_UNARY_FUNC( abs   , ABS   )
	DEFINE_UNARY_FUNC( ceil  , SQRT  )
	DEFINE_UNARY_FUNC( floor , FLOOR )
	DEFINE_UNARY_FUNC( trunc , TRUNC )
	DEFINE_UNARY_FUNC( round , ROUND )
#undef DEFINE_UNARY_FUNC

#define DEFINE_BINARY_OP(op,type) \
	\
	Raster operator op (Raster lhs, Raster rhs) { \
		return Raster( ma_binary(lhs.node,rhs.node,type) ); \
	} \
	\
	Raster operator op (Raster lhs, VariantType rhs) { \
		Raster cnst = full_like(rhs,lhs); \
		return Raster( ma_binary(lhs.node,cnst.node,type) ); \
	} \
	\
	Raster operator op (VariantType lhs, Raster rhs) { \
		Raster cnst =  full_like(lhs,rhs); \
		return Raster( ma_binary(cnst.node,rhs.node,type) ); \
	}

	DEFINE_BINARY_OP( + , ADD )
	DEFINE_BINARY_OP( - , SUB )
	DEFINE_BINARY_OP( * , MUL )
	DEFINE_BINARY_OP( / , DIV )
	DEFINE_BINARY_OP( % , MOD )
	DEFINE_BINARY_OP( == , EQ  )
	DEFINE_BINARY_OP( != , NE  )
	DEFINE_BINARY_OP( <  , LT  )
	DEFINE_BINARY_OP( >  , GT  )
	DEFINE_BINARY_OP( <= , LE  )
	DEFINE_BINARY_OP( >= , GE  )
	DEFINE_BINARY_OP( && , AND )
	DEFINE_BINARY_OP( || , OR  )
	DEFINE_BINARY_OP( &  , bAND )
	DEFINE_BINARY_OP( |  , bOR  )
	DEFINE_BINARY_OP( ^  , bXOR )
	DEFINE_BINARY_OP( << , SHL  )
	DEFINE_BINARY_OP( >> , SHR  )
#undef DEFINE_BINARY_OP

#define DEFINE_BINARY_FUNC(name,type) \
	\
	Raster name(Raster lhs, Raster rhs) { \
		return Raster( ma_binary(lhs.node,rhs.node,type) ); \
	} \
	\
	Raster name(Raster lhs, VariantType rhs) { \
		Raster cnst = full_like(rhs,lhs); \
		return Raster( ma_binary(lhs.node,cnst.node,type) ); \
	} \
	\
	Raster name(VariantType lhs, Raster rhs) { \
		Raster cnst = full_like(lhs,rhs); \
		return Raster( ma_binary(cnst.node,rhs.node,type) ); \
	}

	DEFINE_BINARY_FUNC( max   , MAX2  )
	DEFINE_BINARY_FUNC( min   , MIN2  )
	DEFINE_BINARY_FUNC( atan2 , ATAN2 )
	DEFINE_BINARY_FUNC( pow   , POW   )
	DEFINE_BINARY_FUNC( hypot , HYPOT )
	DEFINE_BINARY_FUNC( fmod  , FMOD  )
#undef DEFINE_BINARY_FUNC

#define DEFINE_DIVERSITY(name,type) \
	Raster name(Raster d1, Raster d2, Raster d3, Raster d4) { \
		std::vector<Node*> vec; \
		vec.push_back(d1.node); \
		vec.push_back(d2.node); \
		vec.push_back(d3.node); \
		vec.push_back(d4.node); \
		return Raster( ma_diversity(vec.data(),vec.size(),type) ); \
	}

	DEFINE_DIVERSITY( variety  , VARI )
	DEFINE_DIVERSITY( majority , MAJO )
	DEFINE_DIVERSITY( minority , MINO )
	DEFINE_DIVERSITY( mean     , MEAN )
#undef DEFINE_DIVERSITY

#define DEFINE_FOCAL(name,type) \
	Raster focal##name(Raster data, const Mask &mask) { \
		VariantType *ptr = const_cast<VariantType*>(&mask.mask[0]); \
		int size = mask.mask.size(); \
		DataSize ds = mask.datasize(); \
		return Raster( ma_focalFunc(data.node,ptr,size,ds,type) ); \
	}

	DEFINE_FOCAL( Sum  , SUM  )
	DEFINE_FOCAL( Prod , PROD )
	DEFINE_FOCAL( Max  , MAX  )
	DEFINE_FOCAL( Min  , MIN  )
#undef DEFINE_FOCAL

#define DEFINE_PERCENT(name,type) \
	Raster name(Raster data, const Mask &mask) { \
		VariantType *ptr = const_cast<VariantType*>(&mask.mask[0]); \
		int size = mask.mask.size(); \
		DataSize ds = mask.datasize(); \
		return Raster( ma_focalPercent(data.node,ptr,size,ds,type) ); \
	}

	DEFINE_PERCENT( percentage , AGE )
	DEFINE_PERCENT( percentile , ILE )
#undef DEFINE_PERCENT

#define DEFINE_REDUCTION(name,type) \
	Raster zonal##name(Raster data) { \
		return Raster( ma_zonalReduc(data.node,type) ); \
	}

	DEFINE_REDUCTION( Sum  , SUM  )
	DEFINE_REDUCTION( Prod , PROD )
	DEFINE_REDUCTION( Max  , MAX  )
	DEFINE_REDUCTION( Min  , MIN  )
#undef DEFINE_REDUCTION

/********************
   Compound Methods
 ********************/

Raster pick(Raster data, std::vector<VariantType> vec) {
	assert(!vec.empty());

	Raster acu = zeros({},vec[0].datatype());
	for (int i=0; i<vec.size(); i++)
		acu = con(data==i, Raster(vec[i]), acu);
	return acu;
}

Raster distance(Raster data, Coord coord) {
	assert(data.numdim() == D2); // Temporary only D2

	auto x = coord[0] - index(data,D1);
	auto y = coord[1] - index(data,D2);
	return sqrt(x*x + y*y + 0.0f);
}

Raster square(Raster data) {
	return data * data;
}
/*
Raster slope(Raster dem, float z_factor, float dist) {
	Array<int> h = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	Array<int> v = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
	Mask H = Mask({3,3}, h);
	Mask V = Mask({3,3}, v);
	Raster x = convolve(dem,H) / 8 / dist;
	Raster y = convolve(dem,V) / 8 / dist;
	Raster z = atan(z_factor * sqrt(x*x + y*y));
	return z;
}

Raster aspect(Raster dem, float dist)
{
	Array<int> h = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	Array<int> v = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
	Mask H = Mask({3,3}, h);
	Mask V = Mask({3,3}, v);
	Raster x = convolve(dem,H) / 8 / dist;
	Raster y = convolve(dem,V) / 8 / dist;
	Raster z1 = con(x!=0,atan2(y,-x),0);
	//Raster z1 = (x!=0) * atan2(y,-x);
	float pi = M_PI;
	z1 = z1 + con(z1<0,2*pi,0.0f);
	//z1 = z1 + (z1 < 0) * 2*M_PI;
	Raster z0 = con(x!=0,0,con(y==0,0,con(y<0,2*pi-pi/2,pi/2)));
	//Raster z0 = (x==0) * ((y>0)*(M_PI/2) + (y<0)*(2*M_PI-M_PI/2) + (y==0)*0);
	return z1 + z0;
}

Raster flowDirection(Raster data) {
	assert( data.datatype() == F32 );
	assert( data.numdim() == D2 );

	Raster dir( FocalFlow::Factory(data.node) );

	return dir;
}

Raster flowDir(Raster data) {
	assert( data.numdim() == D2 );
	assert( data.datatype() == F32 );

	enum {EAST,SOUTHEAST,SOUTH,SOUTHWEST,WEST,NORTHWEST,NORTH,NORTEAST,N_DIR};
	const int offset0[N_DIR] = {1,1,0,-1,-1,-1,0,1};
	const int offset1[N_DIR] = {0,1,1,1,0,-1,-1,-1};

	auto zero8 = zeros({},S8);
	auto one8 = ones({},S8);

	auto maxval = zeros();
	auto maxpos = -one8;
	for (Ctype<U8> dir=EAST; dir<N_DIR; dir++)
	{
		auto of0 = offset0[dir];
		auto of1 = offset1[dir];
	
		auto zdif = data[{0,0}] - data[{of0,of1}];
		auto drop = con(dir%2==0, zdif, zdif/1.414213f);

		auto x = of0 + index(data,D1);
		auto y = of1 + index(data,D2);
		auto out = (x < 0 || x >= data.datasize()[0] || y < 0 || y >= data.datasize()[1]);
		drop = con(out, std::numeric_limits<Ctype<F32>>::max(), drop);

		maxpos = con(drop > maxval, dir, maxpos);
		maxval = con(drop > maxval, drop, maxval);
	}
	auto flow_dir = con(maxpos == -1, zero8, one8 << maxpos);

	return flow_dir;
}
*/
Raster viewshed(Raster data, Coord obs, float obs_h) {
	assert( data.numdim() == D2 );
	assert( data.datatype() == F32 );

	auto shift = data - obs_h;
	auto dist = distance(data,obs);
	auto slope = shift / dist;
	Raster max( ma_radialScan(slope.node,MAX,obs) );
	auto view = (max * dist + obs_h) - data;

	return view;
}

Raster flowAccumulation(Raster data, Raster dir) {
	assert( data.numdim() == D2 );
	assert( dir.numdim() == D2 );
	assert( dir.datatype() == U8 );

	Raster flow( ma_spreadScan(data.node,dir.node,SUM) );
	
	return flow;
}

Raster flowAccuGather(Raster water, Raster dir) { // Gathering nbh
	assert( water.numdim() == D2 );
	assert( dir.numdim() == D2 );
	assert( dir.datatype() == U8 );

	auto dircod = [](Coord c) {
		int dir[3][3] = {{2,4,8},{1,0,16},{128,64,32}};
		return dir[c[1]+1][c[0]+1];
	};

	auto acu = zeros_like(water);
	while (value(zonalSum(water)).convert(B8).get<B8>())
	{
		acu = acu + water;
		auto new_water = zeros();
		for (int y=-1; y<=1; y++) {
			for (int x=-1; x<=1; x++) {
				Coord nbh{x,y};
				if (all(nbh == 0)) continue;
				new_water = con(dir(nbh)==dircod(nbh), new_water+water(nbh), new_water);
			}
		}
		water = new_water;
	}

	return acu;
}

Raster flowAccuSpread(Raster water, Raster dir) { // Spreading-Atomic
	assert( water.numdim() == D2 );
	assert( dir.numdim() == D2 );
	assert( dir.datatype() == U8 );

	auto acu = zeros_like(water);
	while (value(zonalSum(water)).convert(B8).get<B8>()) {
		acu = acu + water;
		water = water(dir,SUM);
	}

	return acu;
}

Raster localMin(Raster *vec, int n) {
	VariantType neutral = std::numeric_limits<Ctype<F32>>::max();
	Raster acu = neutral;
	for (int i=0; i<n; i++)
		acu = min(acu,vec[i]);
	return acu;
}

Raster localOr(Raster *vec, int n) {
	VariantType neutral = false;
	Raster acu = neutral;
	for (int i=0; i<n; i++)
		acu = acu || vec[i];
	return acu;
}

Raster pitFill(Raster dem, Raster stream) {
	assert( dem.datatype() == F32 );

	auto ds = dem.datasize();
	auto idx0 = index(dem,D0);
	auto idx1 = index(dem,D1);
	auto brd0 = idx0 == 0 || idx0 == ds[0]-1;
	auto brd1 = idx1 == 0 || idx1 == ds[1]-1;
	auto border = brd0 || brd1;

	auto orig = dem;
	auto acti = stream || border;
	auto elev = con(acti, orig, 9999); // +inf

	while (value(zonalSum(acti)).convert(B8).get<B8>())
	{
		Raster elev_v[9];
		Raster acti_v[9];
		for (int y=-1; y<=1; y++) { for (int x=-1; x<=1; x++) {
			Coord nbh{x,y};
			auto cond = acti(nbh) && elev(nbh) < elev && elev > orig;
			elev_v[x+3*y] = con(cond, max(elev(nbh),orig), elev );
			acti_v[x+3*y] = con(cond, true, acti);
		} }
		elev = localMin(elev_v,9);
		acti = localOr(acti_v,9);
	}

	return elev;
}

} } // namespace map::detail
