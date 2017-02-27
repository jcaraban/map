/**
 * @file	Cloner.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Cloner.hpp"
#include <algorithm>


namespace map { namespace detail {

Cloner::Cloner(OwnerNodeList &list)
	: new_list(list)
{ }

void Cloner::clear() {
	//visited.clear();
	new_list.clear();
}


NodeList Cloner::clone(NodeList list) {
	clear();
	for (auto node : list)
		node->accept(this);
	NodeList ret_list;
	for (auto &node : new_list)
		ret_list.push_back(node.get());
	return ret_list;
}

void Cloner::visit(Constant *old) {
	DataSize ds = old->datasize();
	DataType dt = old->datatype();
	MemOrder mo = old->memorder();
	BlockSize bs = old->blocksize();
	Node *node = Constant::Factory(old->cnst,ds,dt,mo,bs);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Rand *old) {
	Node *seed = old_hash.find(old->seed())->second;
	DataType dt = old->datatype();
	MemOrder mo = old->memorder();
	Node *node = Rand::Factory(seed,dt,mo);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Index *old) {
	DataSize ds = old->datasize();
	NumDim dim = old->dim;
	MemOrder mo = old->memorder();
	BlockSize bs = old->blocksize();
	Node *node = Index::Factory(ds,dim,mo,bs);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Cast *old) {
	Node *prev = old_hash.find(old->prev())->second;
	DataType new_dt = old->datatype();
	Node *node = Cast::Factory(prev,new_dt);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Unary *old) {
	Node *prev = old_hash.find(old->prev())->second;
	UnaryType type = old->type;
	Node *node = Unary::Factory(prev,type);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Binary *old) {
	Node *lhs = old_hash.find(old->left())->second;
	Node *rhs = old_hash.find(old->right())->second;
	BinaryType type = old->type;
	Node *node = Binary::Factory(lhs,rhs,type);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Conditional *old) {
	Node *cond = old_hash.find(old->cond())->second;
	Node *left = old_hash.find(old->left())->second;
	Node *right = old_hash.find(old->right())->second;
	Node *node = Conditional::Factory(cond,left,right);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Diversity *old) {
	NodeList prev_list;
	for (auto prev : old->prevList()) 
		prev_list.push_back( old_hash.find(prev)->second );
	DiversityType type = old->type;
	Node *node = Diversity::Factory(prev_list,type);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Neighbor *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Coord nbh = old->coord();
	Node *node = Neighbor::Factory(prev,nbh);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(SpreadNeighbor *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Node *dir = old_hash.find(old->dir())->second;
	ReductionType type = old->type;
	Node *node = SpreadNeighbor::Factory(prev,dir,type);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Convolution *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Mask mask = old->mask();
	Node *node = Convolution::Factory(prev,mask);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(FocalFunc *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Mask mask = old->mask();
	ReductionType type = old->type;
	Node *node = FocalFunc::Factory(prev,mask,type);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(FocalPercent *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Mask mask = old->mask();
	PercentType type = old->type;
	Node *node = FocalPercent::Factory(prev,mask,type);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(FocalFlow *old) {
	assert(0); // FocalFlow will be removed
}

void Cloner::visit(ZonalReduc *old) {
	Node *prev = old_hash.find(old->prev())->second;
	ReductionType type = old->type;
	Node *node = ZonalReduc::Factory(prev,type);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(RadialScan *old) {
	Node *prev = old_hash.find(old->prev())->second;
	ReductionType type = old->type;
	Coord start = old->start;
	Node *node = RadialScan::Factory(prev,type,start);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(SpreadScan *old) {
	assert(0); // will be substituted by Loop
}

void Cloner::visit(Loop *old) {
	NodeList prev_list, body_list, feed_in_list, feed_out_list;
	Node *cond;

	// Filling loop.prev_list
	for (auto head : old->headList()) {
		Node *prev = head->prev(); // we want the 'prev'
		Node *newp = old_hash.find(prev)->second;
		prev_list.push_back(newp); // push the 'new_prev'
	}

	// Filling loop.cond
	cond = old_hash.find(old->condition()->prev())->second;

	// Filling loop.body_list
	for (auto body : old->bodyList()) {
		Node *newb = old_hash.find(body)->second;
		body_list.push_back(newb);  // push the 'new_body'
	}

	// Filling loop.feed_in_list
	for (auto fin : old->feed_in_list) {
		auto *head = dynamic_cast<LoopHead*>(fin->prev());
		Node *prev = head->prev(); // jump the 'head'
		Node *newp = old_hash.find(prev)->second;
		feed_in_list.push_back(newp);
	}
	
	// Filling loop.feed_out_list
	for (auto fout : old->feed_out_list) {
		Node *prev = fout->prev(); // we want the 'prev'
		Node *newp = old_hash.find(prev)->second;
		feed_out_list.push_back(newp);
	}

	Node *node = Loop::Factory(prev_list,cond,body_list,feed_in_list,feed_out_list);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );

	// Redirects 'next' nodes to the new 'tail' nodes
	Loop *loop = dynamic_cast<Loop*>(node);
	assert(old->tailList().size() == loop->tailList().size());

	for (int i=0; i<loop->tailList().size(); i++) {
		old_hash.insert( {old->tailList()[i], loop->tailList()[i]} );
		new_hash.insert( {loop->tailList()[i], old->tailList()[i]} );
	}

}

void Cloner::visit(LoopCond *old) {
	// something ?
}

void Cloner::visit(LoopHead *old) {
	// Redirects 'body' / 'feedin' nodes to the 'prev' non-loop node
	Node *node = old_hash.find(old->prev())->second;

	old_hash.insert( {old, node} );
}

void Cloner::visit(LoopTail *old) {
	// something ?
	/*
	// Redirects 'next' nodes to the new 'loop's 'tail' nodes
	Loop *oldl = old->loop();
	Node *newl = old_hash.find(oldl)->second;
	Loop *loop = dynamic_cast<Loop*>(newl);
	
	// Gets old 'tail' pos, find new 'tail' in pos 'i'
	auto list = oldl->tailList();
	int i = std::distance(list.begin(),std::find(list.begin(),list.end(),old));
	Node *node = loop->tailList()[i];

	old_hash.insert( {old, node} );
	*/
}

void Cloner::visit(Feedback *old) {
	if (old->feedOut())
		return; // nothing to do for feedOut case

	// Redirects 'body' nodes to the 'prev' non-loop node
	Node *node = old_hash.find(old->prev())->second;
	//auto *head = dynamic_cast<LoopHead*>(old->prev());
	//Node *prev = head->prev(); // jump the 'head'
	//Node *node = old_hash.find(prev)->second;
	
	old_hash.insert( {old, node} );
}

void Cloner::visit(Access *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Coord coord = old->coord();
	Node *node = Access::Factory(prev,coord);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(LhsAccess *old) {
	Node *left = old_hash.find(old->left())->second;
	Node *right = old_hash.find(old->right())->second;
	Coord coord = old->coord();
	Node *node = LhsAccess::Factory(left,right,coord);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Read *old) {
	//std::string path = old->file()->getFilePath();
	Node *node = Read::Clone(old);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Write *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Node *node = Write::Clone(old,prev);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Scalar *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Node *node = Scalar::Clone(old,prev);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Checkpoint *old) {
	assert(0);

	//new_list.push_back( std::unique_ptr<Node>(node) );
	//old_hash.insert( {old, node} );
}

void Cloner::visit(Stats *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Node *node = Stats::Factory(prev);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

void Cloner::visit(Barrier *old) {
	Node *prev = old_hash.find(old->prev())->second;
	Node *node = Barrier::Factory(prev);

	new_list.push_back( std::unique_ptr<Node>(node) );
	old_hash.insert( {old, node} );
	new_hash.insert( {node, old} );
}

} } // namespace map::detail
