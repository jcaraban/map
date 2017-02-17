/**
 * @file	bindings.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Bridge functions for Python to interact with C++
 *
 * TODO: VariantType / Array4 are passed as pointers while they don't meet the POD / Aggregate constrains
 */

#ifndef MAP_FRONT_BINDINGS_HPP_
#define MAP_FRONT_BINDINGS_HPP_

#include "../util/util.hpp"


namespace map { namespace detail {

struct Node; // forward declaration


extern "C" {

/***************
   Declaration
 ***************/

void ma_setupDevices(const char *plat_name, DeviceType dev, const char *dev_name);

void ma_increaseRef(Node *node);
void ma_decreaseRef(Node *node);

void ma_eval(Node **vec, int num);
VariantType ma_value(Node *node);

//char* ma_nodename(Node *node);
int ma_nodeid(Node *node);

DataTypeEnum ma_datatype(Node *node);
NumDimEnum ma_numdim(Node *node);
MemOrderEnum ma_memorder(Node *node);
DataSize ma_datasize(Node *node);
BlockSize ma_blocksize(Node *node);

/*********
   I / O
 *********/

Node* ma_read(const char *file_path);
int ma_write(Node *node, const char *file_path);

/**************
   Operations
 **************/

Node* ma_constant(VariantType var, DataSize ds, DataTypeEnum dt, MemOrderEnum mo, BlockSize bs);
Node* ma_rand(Node *seed, DataTypeEnum dt, MemOrderEnum mo);
Node* ma_cast(Node *node, DataTypeEnum type);
Node* ma_index(Node *node, NumDimEnum dim);
Node* ma_conditional(Node *cond, Node *lhs, Node *rhs);

Node* ma_unary(Node *node, UnaryEnum type);
Node* ma_binary(Node *lhs, Node* rhs, BinaryEnum type);
Node* ma_diversity(Node **vec, int num, DiversityEnum type);

Node* ma_access(Node *node, Coord coord);
Node* ma_lhsAccess(Node *lhs, Node *rhs, Coord coord);

Node* ma_neighbor(Node *node, Coord coord);
Node* ma_boundedNbh(Node *node, Node *cx, Node *cy);
Node* ma_spreadNeighbor(Node *node, Node *dir, ReductionEnum type);
Node* ma_convolution(Node *node, VariantType *mask_v, int mask_s, DataSize mask_ds);
Node* ma_focalFunc(Node *node, VariantType *mask_v, int mask_s, DataSize mask_ds, ReductionEnum type);
Node* ma_focalPercent(Node *node, VariantType *mask_v, int mask_s, DataSize mask_ds, PercentEnum type);

Node* ma_zonalReduc(Node *node, ReductionEnum type);
Node* ma_radialScan(Node *node, ReductionEnum type, Coord coord);
Node* ma_spreadScan(Node *node, Node *dir, ReductionEnum type);

Node* ma_stats(Node *node);
Node* ma_barrier(Node *node);

/*****************
   Symbolic Loop
 *****************/

void ma_loopStart();
void ma_loopCond(Node *cond);
void ma_loopBody();
void ma_loopAgain();
Node* ma_loopAssemble();
void ma_loopEnd();
void ma_loopAgainTail(Node *loop, Node ***agains, Node ***tails, int *num);

} // extern C

} } // namespace map::detail

#endif
