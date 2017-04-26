/**
 * @file	LocalSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LocalSkeleton.hpp"
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

LocalSkeleton::LocalSkeleton(Version *ver)
	: Skeleton(ver)
{
	indent_count = 2;
}

string LocalSkeleton::generate() {
	tag(); // tag the nodes
	fill(); // fill structures
	compact(); // compact structures

	return versionCode();
}

/***********
   Methods
 ***********/

string LocalSkeleton::versionCode() {
	//// Variables ////
	const int N = ver->task->numdim().toInt();
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

	added.clear();
	for (auto &node : diver) {
		DataType dt = node->prev(0)->datatype();
		if (!added[dt.get()]) {
			add_section( defines_local_diver(dt) );
			added[dt.get()] = true;
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

	add_line( "" );
	
	// Declaring indexing variables
	for (int n=0; n<N; n++) {
		add_line( string("int gc")+n+" = get_local_id("+n+");" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int bc")+n+" = get_global_id("+n+");" );
	}

	add_line( "" );

	// Declaring diversity shared memory
	for (auto &node : diver) {
		int n_arg = node->prevList().size(); 
		int size = n_arg * prod(ver->groupsize());
		string type = node->prev(0)->datatype().ctypeString();
		string elemsa = string("elemSA_") + node->id;
		string cntsa = string("countSA_") + node->id;
		add_line( "local " + type + " " + elemsa + "_group[" + size + "];" );
		add_line( "local int " + cntsa + "_group[" + size + "];" );
		add_line( "local " + type + "* " + elemsa + " = " + elemsa + "_group + " + n_arg + "*(" + local_proj(N) + ");" );
		add_line( "local int* " + cntsa + " = " + cntsa + "_group + " + n_arg + "*(" + local_proj(N) + ");" );
	}
	if (!diver.empty())
		add_line( "" );

	//// Previous to core ////

	//// Core ////

	//// Posterior to core ////

	// Global-if
	add_line( "if ("+global_cond(N)+") {" );
	indent_count++;

	// Adds LOCAL_CORE input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].pos == INPUT_OUTPUT) {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}

	// Adds accumulated 'poscore' to 'all'
	full_code += code_hash[{LOCAL_CORE,1}];

	// Adds LOCAL_CORE output-nodes
	for (auto &node : ver->task->outputList()) {
		add_line( out_var(node) + " = " + var_name(node) + ";" );
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

} } // namespace map::detail
