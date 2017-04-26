/**
 * @file	ZonalSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: the version's variables should be filled during the version creation, not here
 */

#include "ZonalSkeleton.hpp"
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

ZonalSkeleton::ZonalSkeleton(Version *ver)
	: Skeleton(ver)
	, reduc()
{
	indent_count = 3;
}

string ZonalSkeleton::generate() {
	tag(); // tag the nodes
	fill(); // fill structures
	compact(); // compact structures

	// Gives numgroup() as extra_argument
	for (int i=0; i<ver->task->numdim().toInt(); i++)
		ver->extra_arg.push_back( ver->numgroup()[i] );

	return versionCode();
}

/***********
   Methods
 ***********/

string ZonalSkeleton::versionCode() {
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

	std::vector<bool> added(N_DATATYPE,false);
	for (auto &node : ver->task->inputList()) {
		DataType dt = node->datatype();
		if (!added[dt.get()]) {
			add_section( defines_local_type(dt) );
			add_line( "" );
			added[dt.get()] = true;
		}
	}

	std::vector<bool> added_Z(N_REDUCTION,false);
	for (auto &node : reduc) {
		if (!added_Z[node->type.get()]) {
			add_section( defines_zonal_reduc(node->type,node->datatype()) );
			added_Z[node->type.get()] = true;
		}
	}
	add_line( "" );

	// Signature
	add_line( kernel_sign(ver->signature()) );

	// Arguments
	add_line( "(" );
	indent_count++;
	for (auto &node : ver->task->inputList()) {
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
		add_line( string("const int _GS") + n + ", // @" );
	}
	for (int n=0; n<N; n++) {
		comma = (n < N-1) ? "," : "";
		add_line( string("const int GN") + n + comma );
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
	
	// Declaring shared memory
	for (auto &node : reduc) {
		add_line( shared_decl(node,prod(ver->groupsize())) );
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
		add_line( string("int GS")+n+" = 16; // @" );
	}

	add_line( "" );

	//// Previous to core ////
	add_line( "// Previous to ZONAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

	// Adds PRE_ZONAL input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].is(PRE_ZONAL)) {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}

	// Adds accumulated 'precore' to 'all'
	full_code += code_hash[{PRE_ZONAL,1}];

	// Filling shared memory
	for (auto &node : reduc) {
		string svar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string var = var_name(node->prev());
		add_line( svar + " = " + var + ";" );
	}

	indent_count--;
	add_line( "}" );
	add_line( "else" );
	add_line( "{" );
	indent_count++;

	// Filling shared memory, corner cases with neutral element
	for (auto &node : reduc) {
		string svar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string neutral = node->type.neutralString( node->datatype() );
		add_line( svar + " = " + neutral + ";" );
	}

	// Closes global-if and synchronizes
	indent_count--;
	add_line( "}" );
	add_line( "" );

	//// Core ////
	add_line( "// Zonal core\n" );

	// Zonal-loop
	add_line( "for (int i="+group_size_prod(N)+"/2; i>0; i/=2) {" );
	indent_count++;
	add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );
	add_line( "if ("+local_proj_zonal(N)+" < i)" );
	add_line( "{" );
	indent_count++;
	
	// Adds accumulated 'core' to 'all'
	full_code += code_hash[{ZONAL_CORE,1}];

	indent_count--;
	add_line( "}" ); // Closes if
	indent_count--;
	add_line( "}" ); // Closes for
	add_line( "" );

	// Write-if
	add_line( "if ("+local_cond_zonal(N)+")" );
	add_line( "{" );
	indent_count++;

	// Zonal output
	for (auto &node : reduc) {
		string atomic = "atomic" + node->type.toString();
		string var = var_name(node,SHARED) + "["+local_proj_zonal(N)+"]";
		string ovar = string("(global char*)OUT_") + node->id + "+idx_" + node->id;
		add_line( atomic + "( " + ovar + " , " + var + ");" );
	}

	indent_count--;
	add_line( "}" ); // Closes write-if
	add_line( "" );

	//// Posterior to core ////
	add_line( "// Posterior to ZONAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

	// Adds LOCAL_CORE input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].is(LOCAL_CORE)) {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}

	// Adds accumulated 'poscore' to 'all'
	full_code += code_hash[{LOCAL_CORE,1}];

	// Adds LOCAL_CORE output-nodes
	for (auto &node : ver->task->outputList()) {
		if (node->pattern().isNot(ZONAL) && node->numdim() != D0) {
			add_line( out_var(node) + " = " + var_name(node) + ";" );
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

void ZonalSkeleton::visit(ZonalReduc *node) {
	const int N = node->prev()->numdim().toInt();
	string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
	string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

	if (node->type.isOperator())
		add_line( lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";" );
	else if (node->type.isFunction())
		add_line( lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");" );
	else 
		assert(0);

	reduc.push_back(node);
}

void ZonalSkeleton::visit(Summary *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}

void ZonalSkeleton::visit(BlockSummary *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}


} } // namespace map::detail
