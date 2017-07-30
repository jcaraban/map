/**
 * @file	CpuFocalSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "CpuFocalSkeleton.hpp"
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

CpuFocalSkeleton::CpuFocalSkeleton(Version *ver)
	: Skeleton(ver)
	, mask()
	//, conv()
	//, func()
	//, percent()
	//, halo()
{
	indent_count = 2;
	level = 0;
	inner_part = true;
}

string CpuFocalSkeleton::generate() {
	tag();
	fill(); // fill structures
	compact(); // compact structures

	return versionCode();
}

/***********
   Methods
 ***********/

string CpuFocalSkeleton::versionCode() {
	//// Variables ////
	const int N = 2;
	string cond, comma;

	//// Header ////
	indent_count = 0;

	// Includes
	for (auto &incl : include)
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
		bool is_input_focal = isInputOf(node,ver->task->cluster()).is(FOCAL);
		bool is_input_local = isInputOf(node,ver->task->cluster()).is(LOCAL);
		if (!added_F[dt.get()] && is_input_focal) {
			add_section( defines_focal_type(dt) );
			added_F[dt.get()] = true;
			add_line( "" );
		}
		if (!added_L[dt.get()] && (is_input_local || is_input_focal)) {
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
		auto reach = ver->task->inputReach(node,Coord());
		bool extended = prod(reach.datasize()) > 1;
		add_line( in_arg(node,extended) );
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
		add_line( string("const int GS") + n + comma );
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

	// Declaring masks
	for (auto &pair : mask) {
		add_line( mask_decl(pair.first,pair.second) );
	}

	add_line( "" );

	// Declaring indexing variables
	for (int n=0; n<N; n++) {
		add_line( string("int gc")+n+" = get_local_id("+n+");" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int GC")+n+" = get_group_id("+n+");" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int bc")+n+" = get_global_id("+n+");" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int GN")+n+" = get_num_groups("+n+");" );
	}
	// Declaring halos
	for (int n=0; n<N; n++) {
		add_line( std::string("int H")+n + " = " + "1" + ";" );
	}

	add_line( "" );

	//// Previous to core ////
	add_line( "// Previous to FOCAL core\n" );

	if (!code_hash[{PRE_FOCAL,9}].empty())
	{
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

		// Adds PRE_FOCAL input-nodes
		for (auto &node : ver->task->inputList()) {
			if (tag_hash[node].pos == PRE_FOCAL) {
				add_line( var_name(node) + " = " + in_var_focal(node) + ";" );
			}
		}

		// Adds accumulated 'precore' to 'all'
		full_code += code_hash[{PRE_FOCAL,9}];
		
		// Closes load-loop
		indent_count--;
		add_line( "}" );
		// Closes global-if
		indent_count--;
		add_line( "}" );
		// Synchronizes
		add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );
		add_line( "" );
	}

	//// Core ////
	add_line( "// FOCAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

	// Inner-part global-if
	add_line( "if (GC0 > 1 && GC0 < GN0-1 && GC1 > 1 && GC1 < GN1-1)" );
	add_line( "{" );
	indent_count++;

	// Adds accumulated 'core' to 'all'
	full_code += code_hash[{FOCAL_POS,1}];

	indent_count--;
	add_line( "}" ); // Closes inner part

	// Outer-part global-if
	add_line( "if (GC0 == 0 || GC0 == GN0-1 || GC1 == 0 || GC1 == GN1-1)" );
	add_line( "{" );

	// Re-walk the focal nodes for the outer-part code
	inner_part = false;
	code_hash[{FOCAL_POS,1}].clear();

	for (auto node : ver->task->nodeList()) {
		if (node->pattern().is(FOCAL)) {
			node->accept(this);
		}
	}
	// Adds accumulated 'core' to 'all'
	full_code += code_hash[{FOCAL_POS,1}];

	add_line( "}" ); // Closes outer part
	add_line( "" );

	indent_count--;
	add_line( "}" ); // Closes global-if
	add_line( "" );


	//// Posterior to core ////
	add_line( "// Posterior to FOCAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+") {" );
	indent_count++;

	// Adds LOCAL_POS input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].pos == LOCAL_POS) {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
		if (tag_hash[node].pos == PRE_FOCAL && isInputOf(node,ver->task->cluster()).is(LOCAL)) {
			// @ because the computation of the halos does not preserve the scalars
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}


	// Adds accumulated 'poscore' to 'all'
	full_code += code_hash[{LOCAL_POS,1}];

	// Adds LOCAL_POS output-nodes
	for (auto &node : ver->task->outputList()) {
		if (tag_hash[node].pos == LOCAL_POS || tag_hash[node].pos == FOCAL_POS) {
			add_line( out_var(node) + ";" );
		}
	}

	indent_count--;
	add_line( "}" ); // Closes global-if
	indent_count--;
	add_line( "}" ); // Closes kernel body

	//// Printing ////
	std::cout << "***\n" << full_code << "***" << std::endl;

	return full_code;
}

/*********
   Visit
 *********/

void CpuFocalSkeleton::visit(Neighbor *node) {
	const int N = node->numdim().toInt();
	Coord nbh = node->coord();
	string type = node->prev()->datatype().toString();

	string var = var_name(node);
	string load = load = "load_F_" + type + "(VAR_LIST(IN_" + node->prev()->id + "),bc0+"+nbh[0]+",bc1+"+nbh[1]+",BS0,BS1)";

	add_line( var + " = " + load + ";" );
}

void CpuFocalSkeleton::visit(Convolution *node) {
	const int N = node->numdim().toInt();
	string type = node->prev()->datatype().toString();

	string var = var_name(node);
	string mvar = node->mask().datatype().toString() + "L_" + std::to_string(node->id);
	string load;

	if (inner_part)
		load = "load_L_" + type + "(VAR(IN_" + node->prev()->id + "),bc0+i0,bc1+i1,BS0,BS1)";
	else // outer_part
		load = "load_F_" + type + "(VAR_LIST(IN_" + node->prev()->id + "),bc0+i0,bc1+i1,BS0,BS1)";

	indent_count++;
	add_line( var + " = 0;" );

	for (int n=N-1; n>=0; n--) {
		int h = 1;
		string i = string("i") + n;
		add_line( "for (int "+i+"=-H"+n+"; "+i+"<=+H"+n+"; "+i+"++) {" );
		indent_count++;
	}

	for (int n=N-1; n>=0; n--)
		mvar += string("[")+"i"+n+"+H"+n+"]";

	add_line( var + " += " + load + " * " + mvar + ";" );

	for (int n=N-1; n>=0; n--) {
		indent_count--;
		add_line( "}" );
	}
	indent_count--;

	mask.push_back( std::make_pair(node->mask(),node->id) );
}

void CpuFocalSkeleton::visit(FocalFunc *node) {
	const int N = node->numdim().toInt();
	string type = node->prev()->datatype().ctypeString();

	string var = var_name(node);
	string load = "load_F_" + type + "(VAR_LIST(IN_" + node->prev()->id + "),bc0+i0,bc1+i1,BS0,BS1)";

	add_line( var + " = " + node->type.neutralString(node->datatype()) + ";" );

	for (int n=N-1; n>=0; n--) {
		int h = 1;
		string i = string("i") + n;
		add_line( "for (int "+i+"=-H"+n+"; "+i+"<=+H"+n+"; "+i+"++) {" );
		indent_count++;
	}

	if (node->type.isOperator())
		add_line( var + " = " + var + " "+node->type.code()+" " + load + ";" );
	else if (node->type.isFunction())
		add_line( var + " = " + node->type.code()+"(" + var+"," + load +")" + ";" );

	for (int n=N-1; n>=0; n--) {
		indent_count--;
		add_line( "}" );
	}
}

void CpuFocalSkeleton::visit(FocalPercent *node) {
	const int N = node->numdim().toInt();
	string type = node->prev()->datatype().ctypeString();

	string var = var_name(node);
	string pvar = var_name(node->prev());
	string load = "load_F_" + type + "(VAR_LIST(IN_" + node->prev()->id + "),bc0+i0,bc1+i1,BS0,BS1)";

	add_line( var + " = 0;" );

	for (int n=N-1; n>=0; n--) {
		int h = 1;
		string i = string("i") + n;
		add_line( "for (int "+i+"=-H"+n+"; "+i+"<=+H"+n+"; "+i+"++) {" );
		indent_count++;
	}

	add_line( var + " += (" + pvar + " "+node->type.code()+" " + load + ");" );

	for (int n=N-1; n>=0; n--) {
		indent_count--;
		add_line( "}" );
	}

	add_line( var + " /= " + nbh_size(N) + ";" );
}

} } // namespace map::detail
