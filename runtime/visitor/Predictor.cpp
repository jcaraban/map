/**
 * @file    Predictor.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Predictor.hpp"
#include "../Runtime.hpp"
#include "../dag/dag.hpp"


namespace map { namespace detail {

Predictor::Predictor(Group *group)
	: group(group)
{ }

void Predictor::clear() {
	hash.clear();
}

bool Predictor::predict(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	if (!Runtime::getConfig().prediction)
		return false;
	if (group->pattern().is(RADIAL) || group->pattern().is(SPREAD))
		return false;

	clear();
	this->coord = coord;

	// Fills inputs first with 'in_blk'
	for (auto in : in_blk) {
		if (in->holdtype() == HOLD_0) { // When the block is null, looks for the central block
			assert(any(in->key.coord != coord));
			auto pred = [&](const Block *b){ return b->key == Key{in->key.node,coord}; };
			auto it = std::find_if(in_blk.begin(),in_blk.end(),pred);
			assert(it != in_blk.end());
			hash[in->key] = {(*it)->value,(*it)->fixed};
		} else { // HOLD_1 or HOLD_N
			hash[in->key] = {in->value,in->fixed};
		}
	}

	// Iterates the nodes to fill 'value_list' and 'fixed_list'
	for (auto node : full_join(group->nodeList(),group->outputList())) {
		node->accept(this);
		// @ output link nodes are repeated since full_join does not unique
	}

	// Transfer outputs to 'out_blk'
	bool predicted = true;
	for (auto out : out_blk) {
		assert(hash.find(out->key) != hash.end());
		auto vf = hash[out->key];
		if (vf.fixed) {
			out->value = vf.value;
			out->fixed = true;
			out->stats.active = true;
			out->stats.max = vf.value.get();
			out->stats.min = vf.value.get();
			out->key.node->value = vf.value;
		} else {
			predicted = false;
			out->value = VariantType();
			out->fixed = false;
			out->stats = BlockStats();
		}
	}

	// Transfer values to nodes
	for (auto node : group->nodeList()) {
		assert(hash.find({node,coord}) != hash.end());
		auto vf = hash[{node,coord}];
		if (vf.fixed)
			node->value = vf.value;
	}

	return predicted;
}

/*********
   Visit
 *********/

void Predictor::visit(Constant *node) {
	hash[{node,coord}] = {node->cnst,true};
}

void Predictor::visit(Rand *node) {
	hash[{node,coord}] = {{},false};
}

void Predictor::visit(Index *node) {
	hash[{node,coord}] = {{},false};
}

void Predictor::visit(Cast *node) {
	auto prev = hash[{node->prev(),coord}];
	ValFix vf = {{},false};
	if (prev.fixed)
		vf = {prev.value.convert(node->type),true};
	hash[{node,coord}] = vf;
}

void Predictor::visit(Unary *node) {
	auto prev = hash[{node->prev(),coord}];
	ValFix vf = {{},false};
	if (prev.fixed)
		vf = {node->type.apply(prev.value),true};
	hash[{node,coord}] = vf;
}

void Predictor::visit(Binary *node) {
	auto left = hash[{node->left(),coord}];
	auto right = hash[{node->right(),coord}];
	ValFix vf = {{},false};
	if (left.fixed && right.fixed)
		vf = {node->type.apply(left.value,right.value),true};
	else if (node->type == MUL && left.fixed && left.value.isZero())
		vf = {VariantType(0,node->datatype()),true};
	else if (node->type == MUL && right.fixed && right.value.isZero())
		vf = {VariantType(0,node->datatype()),true};
	else if (node->type == GT && left.fixed && left.value.isZero()) // @@
		vf = {VariantType(0,node->datatype()),true};
	hash[{node,coord}] = vf;
}

void Predictor::visit(Conditional *node) {
	auto cond = hash[{node->cond(),coord}];
	auto left = hash[{node->left(),coord}];
	auto right = hash[{node->right(),coord}];
	ValFix vf = {{},false};
	if (cond.fixed) {
		if (cond.value.convert(B8).get<B8>()) {
			if (left.fixed) 
				vf = {left.value,true};
		} else {
			if (right.fixed)
				vf = {right.value,true};
		}
	}
	hash[{node,coord}] = vf;
}

void Predictor::visit(Diversity *node)
{
	for (auto prev : node->prevList()) {
		auto p = hash[{prev,coord}];
		if (!p.fixed) {
			hash[{node,coord}] = {{},false};
			return;
		}
	}
	std::vector<VariantType> vec;
	for (auto prev : node->prevList()) {
		auto p = hash[{prev,coord}];
		vec.push_back(p.value);
	}
	hash[{node,coord}] = {node->type.apply(vec),true};
}

void Predictor::visit(Neighbor *node) {
	auto prev = hash[{node->prev(),coord}];
	auto neig = hash[{node->prev(),coord+node->coord()}];
	ValFix vf = {{},false};
	if (prev.fixed && neig.fixed && prev.value == neig.value)
		vf = {prev.value,true};
	hash[{node,coord}] = vf;
}

void Predictor::visit(SpreadNeighbor *node) {
	assert(0);
}

void Predictor::visit(Convolution *node) {
	auto prev = hash[{node->prev(),coord}];
	auto bt = BinaryType(MUL);
	auto rt = ReductionType(SUM);
	auto acu = rt.neutral(node->datatype());

	if (!prev.fixed) {
		hash[{node,coord}] = {{},false};
		return;
	}
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			assert(hash.find({node->prev(),coord+Coord{x,y}}) != hash.end());
			auto neig = hash[{node->prev(),coord+Coord{x,y}}];
			if (!neig.fixed || prev.value != neig.value) {
				hash[{node,coord}] = {{},false};
				return;
			}
			int proj = (y+1)*3+(x+1);
			auto aux = bt.apply(neig.value,node->mask()[proj]);
			acu = rt.apply(acu,aux);
		}
	}
	hash[{node,coord}] = {acu,true};
}

void Predictor::visit(FocalFunc *node) {
	auto prev = hash[{node->prev(),coord}];
	auto bt = BinaryType(MUL);
	auto rt = ReductionType(node->type);
	auto acu = rt.neutral(node->datatype());

	if (!prev.fixed) {
		hash[{node,coord}] = {{},false};
		return;
	}
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			auto neig = hash[{node->prev(),coord+Coord{x,y}}];
			if (!neig.fixed || prev.value != neig.value) {
				hash[{node,coord}] = {{},false};
				return;				
			}
			int proj = (y+1)*3+(x+1);
			auto aux = bt.apply(neig.value,node->mask()[proj]);
			acu = rt.apply(acu,aux);
		}
	}
	hash[{node,coord}] = {acu,true};
}

void Predictor::visit(FocalPercent *node) {
	assert(0);
}

void Predictor::visit(FocalFlow *node) {
	assert(0);
}

void Predictor::visit(ZonalReduc *node) {
	auto prev = hash[{node->prev(),coord}];
	ValFix vf = {{},false};
	if (prev.fixed)
		vf = {prev.value,true}; // @ only min/max work
	hash[{node,coord}] = vf;
}

void Predictor::visit(RadialScan *node) {
	hash[{node,coord}] = {{},false};
}

void Predictor::visit(SpreadScan *node) {
	assert(0);
}

void Predictor::visit(Access *node) {
	assert(0);
}

void Predictor::visit(LhsAccess *node) {
	assert(0);
}

void Predictor::visit(Read *node) {
	assert(0);
}

void Predictor::visit(Write *node) {
	auto prev = hash[{node->prev(),coord}];
	hash[{node,coord}] = {prev.value,prev.fixed};
}	

void Predictor::visit(Scalar *node) {
	auto prev = hash[{node->prev(),coord}];
	hash[{node,coord}] = {prev.value,prev.fixed};
}

void Predictor::visit(Temporal *node) {
	assert(0);
}

void Predictor::visit(Stats *node) {
	auto prev = hash[{node->prev(),coord}];
	hash[{node,coord}] = {prev.value,prev.fixed};
}

void Predictor::visit(Barrier *node) {
	auto prev = hash[{node->prev(),coord}];
	hash[{node,coord}] = {prev.value,prev.fixed};
}

void Predictor::visit(Checkpoint *node) {
	auto prev = hash[{node->prev(),coord}];
	hash[{node,coord}] = {prev.value,prev.fixed};
}

} } // namespace map::detail
