/**
 * @file	bindings.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "bindings.hpp"
#include "../runtime/dag/dag.hpp"
#include "../runtime/Runtime.hpp"
#include <string>


namespace map { namespace detail {

extern "C" {

void ma_setupDevices(const char *plat_name, DeviceType dev, const char *dev_name) {
	Runtime::getInstance().setupDevices(std::string(plat_name),dev,std::string(dev_name));
}

/**/

void ma_increaseRef(Node *node) {
	if (node != nullptr) {
		//std::cout << "incr " << node->id << " " << node->getName() << std::endl;
		node->increaseRef();
	}
}

void ma_decreaseRef(Node *node) {
	if (node != nullptr) {
		//std::cout << "\tdecr " << node->id << " " << node->getName() << std::endl;
		node->decreaseRef();
	}
}

VariantType get_scalar(Node *node) {
	Node *scalar = Scalar::Factory(node);
	scalar = Runtime::getInstance().addNode(scalar);
	scalar->increaseRef(); // @ keeps the node referenced and alive 
	ma_eval(&scalar,1); // I don't think this can be lazy, maybe with futures
	scalar->decreaseRef();
	auto *sca = dynamic_cast<Scalar*>(scalar);
	return sca->value();
}

void ma_eval(Node **vec, int num) {
	assert(num > 0);
	NodeList list(vec,vec+num);
	Runtime::getInstance().evaluate(list);
}

VariantType ma_value(Node *node) {
	assert(node->numdim() == D0);
	if (node->value.datatype() == NONE_DATATYPE)
		assert(0);//ma_eval(&node,1);
	return node->value;
}

/**/

//char* ma_nodename(Node *node) {
//	return node->getName();
//	// need Node names in a static var, like id.
//}

int ma_nodeid(Node *node) {
	return node->id;
}

/**/

DataTypeEnum ma_datatype(Node *node) {
	return node->datatype().get();
}

NumDimEnum ma_numdim(Node *node) {
	return node->numdim().get();
}

MemOrderEnum ma_memorder(Node *node) {
	return node->memorder().get();
}

DataSize ma_datasize(Node *node) {
	return node->datasize();
}

BlockSize ma_blocksize(Node *node) {
	return node->blocksize();
}

/**/

Node* ma_read(const char *file_path) {
	Node *node = Read::Factory(file_path);
	return Runtime::getInstance().addNode(node);
}

int ma_write(Node *prev, const char *file_path) {
	Node *write = Write::Factory(prev,file_path);
	write = Runtime::getInstance().addNode(write);
	write->increaseRef();
	ma_eval(&write,1); // @ Could be chosen to be LAZY ?
	write->decreaseRef();
	return 0;
}

Node* ma_constant(VariantType var, DataSize ds, DataTypeEnum dt, MemOrderEnum mo, BlockSize bs) {
	Node *node = Constant::Factory(var,ds,dt,mo,bs);
	return Runtime::getInstance().addNode(node);
}

Node* ma_rand(Node *seed, DataTypeEnum dt, MemOrderEnum mo) {
	Node *node = Rand::Factory(seed,dt,mo);
	return Runtime::getInstance().addNode(node);
}

Node* ma_cast(Node *prev, DataTypeEnum type) {
	Node *node = Cast::Factory(prev,type);
	return Runtime::getInstance().addNode(node);
}

Node* ma_index(DataSize ds, NumDimEnum dim, MemOrderEnum mo, BlockSize bs) {
	Node *node = Index::Factory(ds,dim,mo,bs);
	return Runtime::getInstance().addNode(node);
}

Node* ma_conditional(Node *cond, Node *lhs, Node *rhs) {
	Node *node = Conditional::Factory(cond,lhs,rhs);
	return Runtime::getInstance().addNode(node);
}

/**/

Node* ma_unary(Node *prev, UnaryEnum type) {
	Node *node = Unary::Factory(prev,type);
	return Runtime::getInstance().addNode(node);
}

Node* ma_binary(Node *lhs, Node* rhs, BinaryEnum type) {
	Node *node = Binary::Factory(lhs,rhs,type);
	return Runtime::getInstance().addNode(node);
}

Node* ma_diversity(Node **vec, int size, DiversityEnum type) {
	NodeList prev_list(size);
	for (int i=0; i<size; i++)
		prev_list[i] = *vec;
	Node *node = Diversity::Factory(prev_list,type);
	return Runtime::getInstance().addNode(node);
}

/**/

Node* ma_access(Node *prev, Coord coord) {
	Node *node = Access::Factory(prev,coord);
	return Runtime::getInstance().addNode(node);
}

Node* ma_lhsAccess(Node *lhs, Node *rhs, Coord coord) {
	Node *node = LhsAccess::Factory(lhs,rhs,coord);
	return Runtime::getInstance().addNode(node);
}

/* Focal */

Node* ma_neighbor(Node *prev, Coord coord) {
	Node *node = Neighbor::Factory(prev,coord);
	return Runtime::getInstance().addNode(node);
}

Node* ma_boundedNbh(Node *prev, Node *cx, Node *cy) {
	Node *node = BoundedNbh::Factory(prev,cx,cy);
	return Runtime::getInstance().addNode(node);
}

Node* ma_spreadNeighbor(Node *prev, Node *dir, ReductionEnum type) {
	Node *node = SpreadNeighbor::Factory(prev,dir,type);
	return Runtime::getInstance().addNode(node);
}

Node* ma_convolution(Node *prev, VariantType *mask_v, int mask_s, DataSize mask_ds) {
	Array<VariantType> array(mask_s);
	for (int i=0; i<mask_s; i++)
		array[i] = mask_v[i];
	Mask mask(mask_ds,array);
	Node *node = Convolution::Factory(prev,mask);
	return Runtime::getInstance().addNode(node);
}

Node* ma_focalFunc(Node *prev, VariantType *mask_v, int mask_s, DataSize mask_ds, ReductionEnum type) {
	Array<VariantType> array(mask_s);
	for (int i=0; i<mask_s; i++)
		array[i] = mask_v[i];
	Mask mask(mask_ds,array);
	Node *node = FocalFunc::Factory(prev,mask,type);
	return Runtime::getInstance().addNode(node);
}

Node* ma_focalPercent(Node *prev, VariantType *mask_v, int mask_s, DataSize mask_ds, PercentEnum type) {
	Array<VariantType> array(mask_s);
	for (int i=0; i<mask_s; i++)
		array[i] = mask_v[i];
	Mask mask(mask_ds,array);
	Node *node = FocalPercent::Factory(prev,mask,type);
	return Runtime::getInstance().addNode(node);
}

/* Zonal */

Node* ma_zonalReduc(Node *prev, ReductionEnum type) {
	Node *node = ZonalReduc::Factory(prev,type);
	return Runtime::getInstance().addNode(node);
}

/* Radial */

Node* ma_radialScan(Node *prev, ReductionEnum type, Coord coord) {
	Node *node = RadialScan::Factory(prev,type,coord);
	return Runtime::getInstance().addNode(node);
}

/* Spread */

Node* ma_spreadScan(Node *prev, Node *dir, ReductionEnum type) {
	Node *node = SpreadScan::Factory(prev,dir,type);
	return Runtime::getInstance().addNode(node);
}

/* Others */

Node* ma_stats(Node *prev) {
	Node *node = Stats::Factory(prev);
	return Runtime::getInstance().addNode(node);
}

Node* ma_barrier(Node *prev) {
	Node *node = Barrier::Factory(prev);
	return Runtime::getInstance().addNode(node);
}

/* Sym Loop */

void ma_loopStart() {
	Runtime::getLoopAssembler().digestion(true,0,0,0);
}

void ma_loopCond(Node *cond) {
	Runtime::getLoopAssembler().condition(cond);
}

void ma_loopBody() {
	Runtime::getLoopAssembler().digestion(0,true,0,0);
}

void ma_loopAgain() {
	Runtime::getLoopAssembler().digestion(0,0,true,0);
}

Node* ma_loopAssemble () {
	return Runtime::getInstance().loopAssemble();
}

void ma_loopUpdateVars(Node *loop, Node ***oldpy, Node ***newpy, int *num) {
	Runtime::getLoopAssembler().updateVars(loop,oldpy,newpy,num);
}

void ma_loopEnd() {
	Runtime::getLoopAssembler().digestion(0,0,0,true);
}

} // extern C

} } // namespace map::detail
