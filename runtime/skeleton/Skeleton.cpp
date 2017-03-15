/**
 * @file	Skeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: think about making every PatternSkeleton accept a Task of their own pattern
 */

#include "Skeleton.hpp"
#include "LocalSkeleton.hpp"
#include "FocalSkeleton.hpp"
#include "CpuFocalSkeleton.hpp"
#include "ZonalSkeleton.hpp"
#include "FocalZonalSkeleton.hpp"
#include "RadiatingSkeleton.hpp"
#include "SpreadingSkeleton.hpp"
#include "util.hpp"
#include "../Version.hpp"
#include "../task/Task.hpp"
#include <iostream>
#include <functional>


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
}

/***************
   Constructor
 ***************/

Skeleton* Skeleton::Factory(Version *ver) {
	Pattern pat = ver->task->pattern();

	/**/ if ( pat.is(SPECIAL) )
	{
		if ( pat.is(ZONAL) )
		{
			return new ZonalSkeleton(ver);
		}
		else
		{
			//return new SpecialSkeleton(ver);
			assert(false);
		}
	}
	else if ( pat.is(SPREAD) )
	{
		return new SpreadingSkeleton(ver);
	}
	else if ( pat.is(RADIAL) )
	{
		return new RadiatingSkeleton(ver);
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
		assert(0);
	}
}

Skeleton::Skeleton(Version *ver)
	: ver(ver)
	, node_pos(ALL_POS)
	, code()
	, scalar()
	, shared()
	, diver()
	, includes()
	, indent_count(-1)
{ }

/***********
   Methods
 ***********/

void Skeleton::tag(Node *node) {
	bool is_input = is_included(node,ver->task->inputList());
	bool visited = wasVisited(node);
	setVisited(node);
	Pattern pat = node->pattern();
	bool is_core = !is_input && (pat.is(FOCAL) || pat.is(ZONAL) || pat.is(RADIAL) || pat.is(SPREAD));

	if (visited && !(node_pos==PRECORE && tag_hash[node]==POSCORE))
		return;

	if (is_core)
		node_pos = PRECORE;

	if (!is_input) // Recursively goes up if the node is not an input
		for (auto prev : node->prevList())
			tag(prev);

	if (is_core)
		node_pos = CORE;

	tag_hash[node] = node_pos;

	if (is_core)
		node_pos = POSCORE;
}

void Skeleton::fill() {
	visited.clear();
	node_pos = POSCORE;

	// Goes up, node by node, tagging nodes with their relative position to the core
	NodeList list = full_join(ver->task->nodeList(),ver->task->outputList());

	for (auto it=list.rbegin(); it!=list.rend(); it++)
		tag(*it);
	visited.clear();

	// Walks nodes sequentially, filling the code structures
	for (auto node : ver->task->nodeList()) {
		node_pos = tag_hash[node];
		node->accept(this);
	}

	// Fill scalars
	for (auto node : full_join(ver->task->inputList(),ver->task->nodeList()))
		if (!node->isOutput())
			scalar[ node->datatype().get() ].push_back(node->id);

	node_pos = ALL_POS;
}

void Skeleton::compact() {
	//for (int i=F32; i<N_DATATYPE; i++)
	//	sort_unique(scalar[i]);

	sort_unique(shared,node_id_less(),node_id_equal());
	sort_unique(diver,node_id_less(),node_id_equal());
}

string Skeleton::indent() {
	string str = "";
	for (int i=0; i<indent_count; i++)
		str += "\t";
	return str;
}

void Skeleton::add_line(string line) {
	code[node_pos] += indent() + line + "\n";
}

void Skeleton::add_section(string section) {
	code[node_pos] += section;
}

void Skeleton::add_include(string file) {
	// Because the OpenCL kernels are composed at runtime and the user's program could execute anywhere,
	// the includes need to be accessible from any possible path. Copying the header files around with the
	// executables sounds stupid. Defining an absolute path like /usr/include/CL/cl.h seems professiona.
	// The easiest way now is to 1) paste the include into the kernel, 2) cp it to a tmp in the curr dir.

	// TODO
}

/*********
   Visit
 *********/

void Skeleton::visit(Constant *node) {
	add_line( var_name(node) + " = " + node->cnst.toString() + ";" );
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

	includes.push_back("<Random123/philox.h>");
	rand.push_back(node);
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

void Skeleton::visit(Stats *node) {
	assert(0);
}

void Skeleton::visit(Barrier *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}

} } // namespace map::detail
