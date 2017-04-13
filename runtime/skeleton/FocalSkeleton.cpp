/**
 * @file	FocalSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "FocalSkeleton.hpp"
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

FocalSkeleton::FocalSkeleton(Version *ver)
	: Skeleton(ver)
	, mask()
	, conv()
	, func()
	, percent()
	, halo()
{
	indent_count = 2;
	level = 0;
}

void FocalSkeleton::generate() {
	fill(); // fill structures
	compact(); // compact structures

	ver->shared_size = -1;
	ver->group_size = BlockSize{16,16}; // @
	ver->num_group = (ver->task->blocksize() - 1) / ver->groupsize() + 1;
	ver->code = versionCode();
}

/***********
   Methods
 ***********/

void FocalSkeleton::compact() {
	Skeleton::compact();
	//sort_unique(mask,node_id_less(),node_id_equal());
	sort_unique(conv,node_id_less(),node_id_equal());
	sort_unique(func,node_id_less(),node_id_equal());
	sort_unique(percent,node_id_less(),node_id_equal());
}

string FocalSkeleton::versionCode() {
	//// Variables ////
	const int N = 2;
	string cond, comma;

	//// Header ////
	indent_count = 0;

	// Includes
	for (auto &incl : includes)
		add_line( "#include " + incl );
	add_line( "" );
	
	// Adding definitions and utilities
	add_section( defines_local() );
	add_line( "" );
	add_section( defines_focal() );
	add_line( "" );

	std::vector<bool> added_L(N_DATATYPE,false);
	std::vector<bool> added_F(N_DATATYPE,false);
	for (auto &node : ver->task->inputList()) {
		DataType dt = node->datatype();
		if (!added_F[dt.get()] && isInputOf(node,ver->task->group()).is(FOCAL)) {
			add_section( defines_focal_type(dt) );
			added_F[dt.get()] = true;
			add_line( "" );
		}
		if (!added_L[dt.get()] && isInputOf(node,ver->task->group()).is(LOCAL)) {
			add_section( defines_local_type(dt) );
			added_L[dt.get()] = true;
			add_line( "" );
		}
	}
	
	// Signature
	add_line( kernel_sign(ver->signature()) );

	// Arguments
	add_line( "(" );
	indent_count++;
	for (auto &node : ver->task->inputList()) { // keeps the order IN_0, IN_8, ...
		if (tag_hash[node] == PRECORE)
			add_line( "TYPE_VAR_LIST(" + node->datatype().ctypeString() + ",IN_" + node->id + ")," );
		else if (tag_hash[node] == POSCORE)
			add_line( in_arg(node) );
	}
	for (auto &node : ver->task->outputList()) {
		add_line( out_arg(node) );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BS") + n + "," );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BC") + n + "," );
	}
	for (int n=0; n<N; n++) {
		comma = (n < N-1) ? "," : "";
		add_line( string("const int _GS") + n + comma + " // @" );
	}
	indent_count--;
	add_line( ")" );

	add_line( "{" ); // Opens kernel body

	//// Declarations ////
	indent_count++;

	// Declaring scalars
	for (int i=F32; i<N_DATATYPE; i++) {
		if (!scalar[i].empty()) {
			add_line( scalar_decl(scalar[i],static_cast<DataTypeEnum>(i)) );
		}
	}

	BlockSize full_halo = {0,0};
	for (auto h : halo)
		full_halo += h;
	
	// Declaring focal shared memory
	for (auto &node : shared) {
		add_line( shared_decl(node,prod(ver->groupsize()+2*full_halo)) );
	}

	add_line( "" );

	// Declaring indexing variables
	for (int n=0; n<N; n++) {
		add_line( string("int gc")+n+" = get_local_id("+n+");" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int bc")+n+" = get_global_id("+n+");" );
	}
	for (int n=0; n<N; n++) {
		add_line( std::string("int H")+n + " = " + halo_sum(n,halo) + ";" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int GS")+n+" = 16; // @" );
	}

	add_line( "" );

	// Decaring masks
	for (auto &pair : mask) {
		add_line( mask_decl(pair.first,pair.second) );
	}

	add_line( "" );

	//// Previous to core ////
	add_line( "// Previous to FOCAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+") {" );
	indent_count++;

	// Load-loop
	add_line( "for ("+pre_load_loop(N)+")" );
	add_line( "{" );
	indent_count++;

	// Displaced indexing variables
	add_line( "int proj = "+local_proj(N)+" + i*("+group_size_prod(N)+");" );
	add_line( "if (proj >= "+group_size_prod_H(N)+") continue;" );
	for (int n=0; n<N; n++) {
		add_line( string("int gc")+n+" = proj % ("+group_size_prod_H(n+1)+") / "+group_size_prod_H(n)+";" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int bc")+n+" = get_group_id("+n+")*GS"+n+" + gc"+n+" - H"+n+";" );
	}
	add_line( "" );

	// Adds PRECORE input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node] == PRECORE) {
			add_line( var_name(node) + " = " + in_var_focal(node) + ";" );
		}
	}

	// Adds accumulated 'precore' to 'all'
	code[ALL_POS] += code[PRECORE];

	// Filling focal shared memory
	for (auto &node : shared) {
		string svar = var_name(node,SHARED) + "[" + local_proj_focal(N) + "]";
		string var = var_name(node);
		add_line( svar + " = " + var + ";" );
	}
	
	// Closes load-loop
	indent_count--;
	add_line( "}" );
	// Closes global-if
	indent_count--;
	add_line( "}" );
	// Synchronizes
	add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );
	add_line( "" );

	//// Core ////
	add_line( "// FOCAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

	// Adds accumulated 'core' to 'all'
	code[ALL_POS] += code[CORE];

	indent_count--;
	add_line( "}" ); // Closes global-if
	add_line( "" );

	//// Posterior to core ////
	add_line( "// Posterior to FOCAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+") {" );
	indent_count++;

	// Adds POSCORE input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node] == POSCORE) {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
		if (tag_hash[node] == PRECORE && isInputOf(node,ver->task->group()).is(LOCAL)) {
			// @ because the computation of the halos does not preserve the scalars
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}

	// Adds accumulated 'poscore' to 'all'
	code[ALL_POS] += code[POSCORE];

	// Adds POSCORE output-nodes
	for (auto &node : ver->task->outputList()) {
		if (tag_hash[node] == POSCORE || tag_hash[node] == CORE) {
			add_line( out_var(node) + " = " + var_name(node) + ";" );
		}
	}

	indent_count--;
	add_line( "}" ); // Closes global-if
	indent_count--;
	add_line( "}" ); // Closes kernel body

	//// Printing ////
	std::cout << "***\n" << code[ALL_POS] << "***" << std::endl;

	return code[ALL_POS];
}

/*********
   Visit
 *********/

void FocalSkeleton::visit(Neighbor *node) {
	// Adds Neighbor code
	{
		const int N = node->numdim().toInt();
		Coord nbh = node->coord();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_nbh(N,nbh) + "]";

		add_line( var + " = " + svar + ";" );
	}

	if (halo.size() > level) {
		halo[level] = cond(node->halo() > halo[level], node->halo(), halo[level]);
	} else {
		halo.push_back(node->halo());
	}

	shared.push_back(node->prev());
}

void FocalSkeleton::visit(Convolution *node) {
	// Adds convolution code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";
		string mvar = node->mask().datatype().toString() + "L_" + std::to_string(node->id);

		add_line( var + " = 0;" );

		for (int n=N-1; n>=0; n--) {
			int h = node->halo()[n];
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
	
	if (halo.size() > level) {
		halo[level] = cond(node->halo() > halo[level], node->halo(), halo[level]);
	} else {
		halo.push_back(node->halo());
	}

	shared.push_back(node->prev());
	mask.push_back( std::make_pair(node->mask(),node->id) );
}

void FocalSkeleton::visit(FocalFunc *node) {
	// Adds focal code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";

		add_line( var + " = " + node->type.neutralString(node->datatype()) + ";" );

		for (int n=N-1; n>=0; n--) {
			int h = node->halo()[n];
			string i = string("i") + n;
			add_line( "for (int "+i+"=-"+h+"; "+i+"<="+h+"; "+i+"++) {" );
			indent_count++;
		}

		if (node->type.isOperator())
			add_line( var + " = " + var + " "+node->type.code()+" " + svar + ";" );
		else if (node->type.isFunction())
			add_line( var + " = " + node->type.code()+"(" + var+"," + svar+")" + ";" );

		for (int n=N-1; n>=0; n--) {
			indent_count--;
			add_line( "}" );
		}
	}
	
	if (halo.size() > level) {
		halo[level] = cond(node->halo() > halo[level], node->halo(), halo[level]);
	} else {
		halo.push_back(node->halo());
	}

	shared.push_back(node->prev());
	func.push_back(node);
}

void FocalSkeleton::visit(FocalPercent *node) {
	// Adds FocalPercent code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string pvar = var_name(node->prev());
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";

		add_line( var + " = 0;" );

		for (int n=N-1; n>=0; n--) {
			string i = string("i") + n;
			add_line( "for (int "+i+"=0; "+i+"<"+(node->halo()[n]*2+1)+"; "+i+"++) {" );
			indent_count++;
		}

		add_line( var + " += (" + pvar + " "+node->type.code()+" " + svar + ");" );

		for (int n=N-1; n>=0; n--) {
			indent_count--;
			add_line( "}" );
		}

		add_line( var + " /= " + nbh_size(N) + ";" );
	}
	
	shared.push_back(node->prev());
	percent.push_back(node);
}

} } // namespace map::detail
