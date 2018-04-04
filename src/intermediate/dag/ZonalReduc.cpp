/**
 * @file	ZonalReduc.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#include "ZonalReduc.hpp"
#include "../../visitor/Visitor.hpp"
#include <functional>


namespace map { namespace detail {

// Internal declarations

ZonalReduc::Content::Content(ZonalReduc *node) {
	prev = node->prev();
	type = node->type;
}

bool ZonalReduc::Content::operator==(const Content& k) const {
	return (prev==k.prev && type==k.type);
}

std::size_t ZonalReduc::Hash::operator()(const Content& k) const {
	return std::hash<Node*>()(k.prev) ^ std::hash<int>()(k.type.get());
}

// Factory

Node* ZonalReduc::Factory(Node *prev, ReductionType type) {
	assert(prev != nullptr);
	assert(prev->numdim() != D0);

	DataSize ds = DataSize(0); // == D0
	DataType dt = prev->datatype();
	MemOrder mo = prev->memorder();
	BlockSize bs = BlockSize(0);
	GroupSize gs = GroupSize(0);
	MetaData meta(ds,dt,mo,bs,gs);

	return new ZonalReduc(meta,prev,type);
}

Node* ZonalReduc::clone(const std::unordered_map<Node*,Node*> &other_to_this) {
	return new ZonalReduc(this,other_to_this);
}

// Constructors

ZonalReduc::ZonalReduc(const MetaData &meta, Node *prev, ReductionType type) : Node(meta) {
	prev_list.reserve(1);
	this->addPrev(prev);
	prev->addNext(this);

	this->type = type;
	//this->value = type.neutral(datatype());
	
	this->in_spatial_reach = Mask(prev->numdim().unitVec(),true);
	this->out_spatial_reach = Mask(numdim().unitVec(),true); // @ shall be empty ?
}

ZonalReduc::ZonalReduc(const ZonalReduc *other, const std::unordered_map<Node*,Node*> &other_to_this)
	: Node(other,other_to_this)
{
	this->type = other->type;
}

// Methods

void ZonalReduc::accept(Visitor *visitor) {
	visitor->visit(this);
}

std::string ZonalReduc::shortName() const {
	return "ZonalReduc";
}

std::string ZonalReduc::longName() const {
	auto tstr = type.toString();
	auto pid = std::to_string(prev()->id);
	std::string str = "ZonalReduc {" + tstr + "," + pid + "}";
	return str;
}

std::string ZonalReduc::signature() const {
	std::string sign = "";
	sign += classSignature();
	sign += prev()->numdim().toString();
	sign += prev()->datatype().toString();
	sign += type.toString();
	return sign;
}

Node* ZonalReduc::prev() const {
	return prev_list[0];
}

// Compute

VariantType ZonalReduc::initialValue() const {
	return type.neutral(datatype());
}

void ZonalReduc::updateValue(VariantType value) {
	//node->type.atomic(node->value,blk->value); // @
	this->value = type.apply(this->value,value);
}

void ZonalReduc::computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash) {
	auto *node = this;
	ValFix vf = ValFix();

	auto prev = hash.find({node->prev(),coord})->second;

	if (prev.fixed) {
		Ctype<F64> num = prod(blocksize());
		Ctype<F64> val = prev.value.convert(F64).get<F64>();
		// @ sure about using F64 ?

		switch (node->type.get()) {
			case SUM:  vf = ValFix(VariantType( val * num, datatype() )); break;
			case PROD: vf = ValFix(VariantType( std::pow(val,num), datatype() )); break;
			case rAND: vf = ValFix(prev.value); break;
			case rOR:  vf = ValFix(prev.value); break;
			case MAX:  vf = ValFix(prev.value); break;
			case MIN:  vf = ValFix(prev.value); break;
			default: assert(0);
		}
	}
	hash[{node,coord}] = vf;
}

} } // namespace map::detail
