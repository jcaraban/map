/**
 * @file	RadialSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: the version's variables should be filled during the version creation, not here
 */

#include "RadialSkeleton.hpp"
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

RadialSkeleton::RadialSkeleton(Version *ver)
	: Skeleton(ver)
	, radia()
{
	indent_count = 4;
}

string RadialSkeleton::generate() {
	tag(); // tag the nodes
	fill(); // fill structures
	compact(); // compact structures

	Direction fst, snd;
	RadialCase rcase = str2radia(ver->detail);
	radia2dir(rcase,fst,snd);

	return versionCode(rcase,fst,snd);
}

/***********
   Methods
 ***********/

string RadialSkeleton::versionCode(RadialCase rcase, Direction fst, Direction snd) {
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
	add_section( defines_radial() );
	add_line( "" );

	std::vector<bool> added_L(N_DATATYPE,false);
	for (auto &node : ver->task->inputList()) {
		DataType dt = node->datatype();
		if (!added_L[dt.get()]) {
			add_section( defines_local_type(dt) );
			added_L[dt.get()] = true;
			add_line( "" );
		}
	}

	std::vector<bool> added_R(N_DATATYPE,false);
	for (auto &node : ver->task->nodeList()) {
		DataType dt = node->datatype();
		if (!added_R[dt.get()] && node->pattern().is(RADIAL)) {
			add_section( defines_radial_type(dt) );
			added_R[dt.get()] = true;
			add_line( "" );
		}
	}
	
	// Signature
	add_line( kernel_sign(ver->signature()) );

	// Arguments
	add_line( "(" );
	indent_count++;
	for (auto &node : ver->task->inputList()) {
		add_line( in_arg(node) );
	}
	for (auto &node : ver->task->outputList()) {
		if (node->pattern().is(RADIAL))
			add_line( "TYPE_VAR_LIST(" + node->datatype().ctypeString() + ",OUT_" + node->id + ")," );
		else //if (tag_hash[node].is(INPUT_OUTPUT))
			add_line( out_arg(node) );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BS") + n + "," );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BC") + n + "," );
	}
	//for (int n=0; n<N; n++) {
	//	add_line( string("const int GS") + n + "," );
	//}
	add_line( "const int GS," );
	for (int n=0; n<N; n++) {
		comma = (n < N-1) ? "," : "";
		add_line( string("const int sc") + n + comma );
	}
	indent_count--;
	add_line( ")" );

	add_line( "{" ); // Opens kernel body

	//// Declarations ////

	// Declaring local / global ids
	indent_count++;
	
	// Declaring scalars
	for (int i=F32; i<N_DATATYPE; i++) {
		if (!scalar[i].empty()) {
			add_line( scalar_decl(scalar[i],static_cast<DataTypeEnum>(i)) );
		}
	}

	// Declaring Radial shared memory
	for (auto &node : radia) {
		add_line( shared_decl(node,prod(ver->groupsize())+1) );
	}

	// Adding Constant variables
	add_section( defines_radial_const(fst,snd) );

	// Adding indexing variables
	add_section( defines_radial_idx() );

	//// Previous to core ////
	indent_count += 2;
	add_line( "" );
	add_line( "// Previous to RADIAL core\n" );

	add_line( "if (cond)" ); // Global-if
	add_line( "{" );
	indent_count++;

	// Adds PRE_RADIAL input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].is(PRE_RADIAL)) {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}

	// Adds accumulated 'precore' to 'all'
	full_code += code_hash[{PRE_RADIAL,1}];

	indent_count--;
	add_line( "}" ); // Closes global-if
	add_line( "" );

	//// Core ////
	add_line( "// RADIAL core\n" );

	add_line( "if (dif0 == 0 && dif1 == 0)" );
	add_line( "{" );
	indent_count++;

	// Center case with neutral element
	for (auto &node : radia) {
		string neutral = node->type.neutralString( node->datatype() );
		string svar = var_name(node,SHARED) + "[gc+1]";
		add_line( svar + " = " + neutral + ";" );
		svar = var_name(node,SHARED) + "[gc]";
		add_line( svar + " = " + neutral + ";" );
	}

	indent_count--;
	add_line( "}" );
	add_line( "else if (cond)" ); // Global-if
	add_line( "{" );
	indent_count++;
	add_line( "if (i0 == 0) {" ); // Local-if
	indent_count++;
	add_line( "int c0 = bc0 - fst_unit0;" );
	add_line( "int c1 = bc1 - fst_unit1;" );

	// Filling Radial shared memory
	for (auto &node : radia) {
		string svar = var_name(node,SHARED) + "[gc+1]";
		string type = node->datatype().toString();
		string load = "load_R_" + type + "(VAR_LIST(OUT_" + node->id + "),c0,c1,BS0,BS1)";
		add_line( svar + " = " + load + ";" );
	}

	indent_count--;
	add_line( "}" ); // Closes local-if
	add_line( "if (gc == 0) {" ); // Local-if
	indent_count++;
	add_line( "int c0 = bc0 - fst_unit0 - snd_unit0;" );
	add_line( "int c1 = bc1 - fst_unit1 - snd_unit1;" );

	// Filling corner
	for (auto &node : radia) {
		string svar = var_name(node,SHARED) + "[gc]";
		string type = node->datatype().toString();
		string load = "load_R_" + type + "(VAR_LIST(OUT_" + node->id + "),c0,c1,BS0,BS1)";
		add_line( svar + " = " + load + ";" );
	}

	indent_count--;
	add_line( "}" ); // Closes local-if
	indent_count--;
	add_line( "}" ); // Closees global-if

	add_line( "barrier(CLK_LOCAL_MEM_FENCE);" ); // Synchronizes
	add_line( "" );
	add_line( "if (cond)" ); // Global-if
	add_line( "{" );
	indent_count++;

	// Adds accumulated 'core' to 'all'
	full_code += code_hash[{RADIAL_CORE,1}];

	indent_count--;
	add_line( "}" ); // Closes global-if
	add_line( "" );

	add_line( "if (cond)" ); // Global-if
	add_line( "{" );
	indent_count++;

	// Radial output
	for (auto &node : ver->task->outputList()) {
		if (node->pattern().is(RADIAL)) {
			string ovar = string("OUT_") + node->id + "[" + global_proj(N) + "]";
			string var = var_name(node);
			add_line( ovar + " = " + var + ";" );
		}
	}

	indent_count--;
	add_line( "}" ); // Closes global-if
	add_line( "" );

	//// Posterior to core ////
	add_line( "// Posterior to RADIAL core\n" );

	add_line( "if (cond)" );
	add_line( "{" ); // Global-if
	indent_count++;

	// Adds LOCAL_CORE input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].is(INPUT_OUTPUT)) {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}

	// Adds accumulated 'poscode' to 'all'
	full_code += code_hash[{LOCAL_CORE,1}];

	// Adds LOCAL_CORE output-nodes
	for (auto &node : ver->task->outputList()) {
		if (node->pattern().isNot(RADIAL)) {
			add_line( out_var(node) + " = " + var_name(node) + ";" );
		}
	}

	indent_count--;
	add_line( "}" ); // Closes global-if
	indent_count--;
	add_line( "} // end 1st dim FOR" );
	indent_count--;
	add_line( "} // end 2st dim FOR" );
	indent_count--;
	add_line( "}" ); // Closes kernel body

	//// Printing ////
	if (rcase == 0)
		std::cout << "***\n" << full_code << "***" << std::endl;

	return full_code;
}

/*********
   Visit
 *********/

void RadialSkeleton::visit(RadialScan *node) {
	// Adds Radial code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string ivar = var_name(node->prev());
		string avar = var_name(node,SHARED) + "[gc+1]";
		string bvar = var_name(node,SHARED) + "[gc]";
		string data_type = node->datatype().toString();

		add_line( var + " = LIT_" + data_type + "(" + avar + "," + bvar + ");" );
		add_line( "if (isfinite(" + ivar + "))" );
		indent_count++;

		if (node->type.isOperator())
			add_line( var + " = " + var + " " + node->type.code() + " " + ivar + ";" );
		else if (node->type.isFunction())
			add_line( var + " = " + node->type.code() + "(" + var + ", " + ivar + ");" );
		else 
			assert(0);

		indent_count--;
		add_line( avar + " = " + var + ";" );
	}

	radia.push_back(node);	
}

} } // namespace map::detail
