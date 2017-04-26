/**
 * @file	Skeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Skeleton.hpp"
#include "LocalSkeleton.hpp"
#include "FocalSkeleton.hpp"
#include "CpuFocalSkeleton.hpp"
#include "ZonalSkeleton.hpp"
#include "FocalZonalSkeleton.hpp"
#include "RadialSkeleton.hpp"
#include "SpreadSkeleton.hpp"
#include "util.hpp"
#include "../Version.hpp"
#include "../task/Task.hpp"
#include <iostream>
#include <functional>


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
}

/***********
   SkelTag
 ***********/

void SkelTag::add(SkelPos pos) {
	this->pos = static_cast<SkelPos>(static_cast<int>(this->pos) | static_cast<int>(pos));
};

bool SkelTag::is(SkelPos pos) const {
	return (this->pos & pos) == pos;
};

bool SkelTag::operator==(SkelTag tag) const {
	return this->pos == tag.pos && this->pds == tag.pds;
};

bool SkelTag::operator<(SkelTag tag) const {
	if (this->pds > tag.pds)
		return 1;
	else if (this->pds < tag.pds)
		return 0;
	else if (this->pos < tag.pos)
		return 1;
	else // this->pos > tag.pos
		return 0;
};

std::size_t SkelTag::Hash::operator()(const SkelTag& k) const {
	return ((size_t)k.pos & 0x00000000ffffffff) | ((size_t)k.pds << 32);
}

/***************
   Constructor
 ***************/

Skeleton* Skeleton::Factory(Version *ver) {
	Pattern pat = ver->task->pattern();

	/**/ if ( pat.is(STATS) )
	{
		if ( pat.is(ZONAL) )
		{
			return new ZonalSkeleton(ver);
		}
		else
		{
			//return new StatsSkeleton(ver);
			assert(false);
		}
	}
	else if ( pat.is(SPREAD) )
	{
		return new SpreadSkeleton(ver);
	}
	else if ( pat.is(RADIAL) )
	{
		return new RadialSkeleton(ver);
	}
	else if ( pat.is(FOCAL+ZONAL) )
	{
		return new FocalZonalSkeleton(ver);
	}
	else if ( pat.is(ZONAL) )
	{
		return new ZonalSkeleton(ver);
	}
	else if ( pat.is(FOCAL) )
	{
		/**/ if ( ver->deviceType() == DEV_CPU )
		{
			return new CpuFocalSkeleton(ver);
		}
		else if ( ver->deviceType() == DEV_GPU)
		{
			return new FocalSkeleton(ver);
		}
		else
			assert(0);
	}
	else if ( pat.is(LOCAL) )
	{
		return new LocalSkeleton(ver);
	}
	else {
		return new Skeleton(ver);
	}
}

Skeleton::Skeleton(Version *ver)
	: ver(ver)
	, indent_count(-1)
{ }

string Skeleton::generate() {
	tag(); // tag the nodes
	fill(); // fill structures
	compact(); // compact structures

	return versionCode();
}

/***********
   Methods
 ***********/

void Skeleton::tag() {
	// Puts in + body + out nodes into 'all_list', in order
	auto all_list = full_join(ver->task->inputList(),ver->task->nodeList());
	all_list = full_unique_join(all_list,ver->task->outputList());

	// Walks nodes backward and accumulate their 'tag = { SkelPos , prod_data_size }'
	for (auto it=all_list.rbegin(); it!=all_list.rend(); it++) {
		Node *node = *it;
		Mask reach = ver->task->inputReach(node,Coord());
		Pattern pat = node->pattern();
		SkelTag tag;

		tag.pds = prod(reach.datasize());
		tag.pos = NONE_SKEL_POS;

		auto next_inside = inner_join(node->nextList(),ver->task->nodeList());
		for (auto next : next_inside) {
			auto next_tag = tag_hash.find(next)->second;
			// Augments reach
			tag.pds = std::max(tag.pds,next_tag.pds);
			// Augments tag
			if (next_tag.is(PRE_FOCAL) || next_tag.is(FOCAL_CORE))
				tag.add(PRE_FOCAL);
			if (next_tag.is(PRE_ZONAL) || next_tag.is(ZONAL_CORE))
				tag.add(PRE_ZONAL);
			if (next_tag.is(PRE_RADIAL) || next_tag.is(RADIAL_CORE))
				tag.add(PRE_RADIAL);
		}
		
		if (is_included(node,ver->task->inputList()))
			tag.add(INPUT_OUTPUT);
		else if (node->pattern().is(FOCAL))
			tag.add(FOCAL_CORE);
		else if (node->pattern().is(ZONAL))
			tag.add(ZONAL_CORE);
		else if (node->pattern().is(RADIAL))
			tag.add(RADIAL_CORE);
		else if (tag.pos == NONE_SKEL_POS)
			tag.add(LOCAL_CORE);
		
		tag_hash[node] = tag;
	}

	// Register the different tag categories in order from max to min 'pds'

	for (auto it=tag_hash.begin(); it!=tag_hash.end(); it++)
		tag_vec.push_back(it->second);
	sort_unique(tag_vec);
	
}

void Skeleton::fill() {
	code_hash.clear();
	full_code.clear();

	// Reserves some decent length for the strings
	const int avg_skel_len = 4096;	
	for (auto tag : tag_vec)
		code_hash[tag].reserve(avg_skel_len);

	// Walks nodes sequentially, filling the code structures
	for (auto node : ver->task->nodeList()) {
		curr_tag = tag_hash.find(node)->second;
		node->accept(this);
	}

	// Fill scalars
	for (auto node : full_join(ver->task->inputList(),ver->task->nodeList()))
		scalar[ node->datatype().get() ].push_back(node->id);

	curr_tag = SkelTag{ALL_SKEL_POS,0};
}

void Skeleton::compact() {
	//for (int i=F32; i<N_DATATYPE; i++)
	//	sort_unique( scalar[i] );

	sort_unique(shared,node_id_less(),node_id_equal());
	sort_unique(diver,node_id_less(),node_id_equal());
}

std::string Skeleton::versionCode() {
	assert(0);

	... continue ... SKEL Unification, then TASK Unification
}

string Skeleton::indent() {
	string str = "";
	for (int i=0; i<indent_count; i++)
		str += "\t";
	return str;
}

void Skeleton::add_line(string line) {
	if (curr_tag.pos == ALL_SKEL_POS)
		full_code += indent() + line + "\n";
	else
		code_hash[curr_tag] += indent() + line + "\n";
}

void Skeleton::add_section(string section) {
	full_code += section;
}

void Skeleton::add_include(string file) {
	// Because the OpenCL kernels are composed at runtime and the user's program could execute anywhere,
	// the includes need to be accessible from any possible path. Copying the header files around with the
	// executables sounds stupid. Defining an absolute path like /usr/include/CL/cl.h seems more elegant.
	// The easiest way now is to 1) paste the include into the kernel, 2) cp it to a tmp in the curr dir.

	// TODO
}

/*********
   Visit
 *********/

void Skeleton::visit(Constant *node) {
	add_line( var_name(node) + " = " + node->cnst.toString() + ";" );
}

void Skeleton::visit(Index *node) {
	string var = var_name(node);
	if (node->dim == D1)
		add_line( var + " = BC0*BS0+bc0;" );
	else if (node->dim == D2)
		add_line( var + " = BC1*BS1+bc1;" );
	else if (node->dim == D3)
		add_line( var + " = BC2*BS2+bc2;" );
	else
		assert(0);
}

void Skeleton::visit(Identity *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());
	add_line( var + " = " + pvar+ + ";" );
}

void Skeleton::visit(Rand *node) {
	std::array<std::string,N_DATATYPE> max_vec = {"/0","UCHAR_MAX","USHRT_MAX","/0","UINT_MAX","/0","/0","/0","ULONG_MAX"};
	int N = node->numdim().toInt();
	DataType type = node->datatype();
	string var = var_name(node);
	string ctr = std::string("ctr_") + node->id;
	string key = std::string("key_") + node->id;
	string bit = (type.sizeOf() > 4) ? "64" : "32";
	string ut = type.toUnsigned().ctypeString();
	string max_str = max_vec[ type.sizeOf() ];

	add_line( "philox2x"+bit+"_ctr_t ctr_"+std::to_string(node->id)+" = {{"+var_name(node->seed())+"}};" );
	add_line( "philox2x"+bit+"_key_t key_"+std::to_string(node->id)+" = {{"+total_proj(N)+"}};" );

	add_line( ctr + " = philox2x"+bit+"("+ctr+","+key+");" );
	if (type.isUnsigned() || type.isSigned()) {
		add_line( var + " = *("+type.ctypeString()+"*)& " + ctr +";" );
	} else { // isFloating()
		add_line( var + " = *("+ut+"*)& " + ctr +" / ("+type.ctypeString()+")"+max_str+";" );
	}

	include.push_back("<Random123/philox.h>");
	rand.push_back(node);
}

void Skeleton::visit(Cast *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());
	add_line( var + " = " + "("+node->type.ctypeString()+") " + pvar+ + ";" );
}

void Skeleton::visit(Unary *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());

	if (node->type.isOperator())
		add_line( var + " = " + node->type.code() + pvar+ + ";" );
	else if (node->type.isFunction())
		add_line( var + " = " + node->type.code()+"(" + pvar+")" + ";" );
	else 
		assert(0);
}

void Skeleton::visit(Binary *node) {
	string var = var_name(node);
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());

	if (node->type.isOperator())
		add_line( var + " = " + lvar + " "+node->type.code()+" " + rvar + ";" );
	else if (node->type.isFunction())
		add_line( var + " = " + node->type.code()+"(" + lvar+"," + rvar+")" + ";" );
	else 
		assert(0);
}

void Skeleton::visit(Conditional *node) {
	string var = var_name(node);
	string cvar = var_name(node->cond());
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());
	add_line( var + " = (" + cvar + ") ? " + lvar + " : " + rvar + ";" );
}

void Skeleton::visit(Diversity *node) {
	DataType in_type = node->prev(0)->datatype(); // Type of the inputs
	string type_str = in_type.ctypeString();
	int n_arg = node->prevList().size();
	
	string var = var_name(node);
	string elemsa = string("elemSA_") + node->id;
	string cntsa = string("countSA_") + node->id;
	string num = string("num_") + node->id;

	add_line( "int " + num + " = 0;" );

	for (int i=0; i<n_arg; i++) {
		string pvar = var_name(node->prev(i));
		add_line( "addElem_"+type_str+"("+pvar+","+elemsa+","+cntsa+",&"+num+");" );
	}

	if (node->type == VARI)
		add_line( var + " = " + num + ";" );
	else if (node->type == MAJO)
		add_line( var + " = " + "majorElem_"+type_str+"("+elemsa+","+cntsa+","+num+")" + ";" );
	else if (node->type == MINO)
		add_line( var + " = " + "minorElem_"+type_str+"("+elemsa+","+cntsa+","+num+")" + ";" );
	else if (node->type == MEAN)
		add_line( var + " = " + "meanElem_"+type_str+"("+elemsa+","+cntsa+","+num+")" + ";" );
	else
		assert(0);

	diver.push_back(node);
}

void Skeleton::visit(Neighbor *node) {
	// Adds Neighbor code
	{
		const int N = node->numdim().toInt();
		Coord nbh = node->coord();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_nbh(N,nbh) + "]";

		add_line( var + " = " + svar + ";" );
	}

	shared.push_back(node->prev());
}

void Skeleton::visit(BoundedNeighbor *node) {
	assert(0);
}

void Skeleton::visit(SpreadNeighbor *node) {
	assert(0);
}

void Skeleton::visit(Convolution *node) {
	// Adds convolution code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";
		string mvar = node->mask().datatype().toString() + "L_" + std::to_string(node->id);

		add_line( var + " = 0;" );

		for (int n=N-1; n>=0; n--) {
			int h = 1;
			string i = string("i") + n;
			add_line( "for (int "+i+"=-"+h+"; "+i+"<="+h+"; "+i+"++) {" );
			indent_count++;
		}

		for (int n=N-1; n>=0; n--)
			mvar += string("[")+"i"+n+"+H"+n+"]";

		add_line( var + " += " + svar + " * " + mvar + ";" );

		for (int n=N-1; n>=0; n--) {
			indent_count--;
			add_line( "}" );
		}
	}
	
	shared.push_back(node->prev());
	//mask.push_back( std::make_pair(node->mask(),node->id) );
}

void Skeleton::visit(FocalFunc *node) {
	assert(0);
}

void Skeleton::visit(FocalPercent *node) {
	assert(0);
}

void Skeleton::visit(ZonalReduc *node) {
	const int N = node->prev()->numdim().toInt();
	string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
	string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

	if (node->type.isOperator())
		add_line( lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";" );
	else if (node->type.isFunction())
		add_line( lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");" );
	else 
		assert(0);
}

void Skeleton::visit(RadialScan *node) {
	assert(0);
}

void Skeleton::visit(SpreadScan *node) {
	assert(0);
}

void Skeleton::visit(LoopCond *node) {
	assert(0);
}

void Skeleton::visit(LoopHead *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());
	add_line( var + " = " + pvar+ + ";" );
}


void Skeleton::visit(LoopTail *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());
	add_line( var + " = " + pvar+ + ";" );
}

void Skeleton::visit(Merge *node) {
	assert(0);
}

void Skeleton::visit(Switch *node) {
	assert(0);
}

void Skeleton::visit(LhsAccess *node) {
	string var = var_name(node);
	Coord coord = node->cell_coord;
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());
	int N = node->numdim().toInt();
	add_line( var + " = (" + equal_coord_cond(N,coord) + ") ? " + rvar + " : " + lvar + ";" );
}

void Skeleton::visit(Access *node) {
	string var = var_name(node);
	Coord coord = node->cell_coord;
	string pvar = var_name(node->prev());
	int N = node->prev()->numdim().toInt();

	add_line( "if (" + equal_coord_cond(N,coord) + ") " + var + " = " + pvar + ";" );
}

void Skeleton::visit(Read *node) {
	assert(0);
}

void Skeleton::visit(Write *node) {
	assert(0);
}

void Skeleton::visit(Scalar *node) {
	assert(0);
}

void Skeleton::visit(Temporal *node) {
	// nothing to do
}

void Skeleton::visit(Checkpoint *node) {
	assert(0);
}

void Skeleton::visit(Barrier *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}

void Skeleton::visit(Summary *node) {
	assert(0);
}

void Skeleton::visit(BlockSummary *node) {
	assert(0);
}

} } // namespace map::detail
