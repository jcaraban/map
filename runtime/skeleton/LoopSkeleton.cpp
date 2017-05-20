/**
 * @file	LoopSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LoopSkeleton.hpp"
#include "util.hpp"
#include "../Version.hpp"
#include "../task/Task.hpp"
#include <iostream>
#include <functional>


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
}

LoopSkeleton::LoopSkeleton(Version *ver)
	: Skeleton(ver)
{ }

string LoopSkeleton::generate() {
	tag(); // tag the nodes
	fill(); // fill structures
	compact(); // compact structures

	return versionCode();
}

std::string LoopSkeleton::versionCode() {
	//// Variables ////
	const int N = ver->task->numdim().toInt();
	Pattern pattern = ver->task->group()->pattern();
	string cond, comma;

	//// Defines ////
	indent_count = 0;

	// Includes
	for (auto &incl : include)
		add_line( "#include " + incl );
	add_line( "" );

	// Definitions, Utilities
	add_section( defines_local() );
	add_line( "" );
	

	bool local_types[N_DATATYPE] = {};

	auto any_true = [](bool array[], int num){
		return std::any_of(array,array+num,[](bool b) { return b; });
	};

	for (auto node : ver->task->inputList()) {
		DataType dt = node->datatype();
		local_types[dt.get()] = true;
	}

	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		if (local_types[dt])
			add_section( defines_local_type(dt) );
	if (any_true(local_types,N_DATATYPE))
		add_line( "" );

	add_section( defines_reduc_type(rOR,U8) );
	add_line( "" );

	//// Header ////

	// Signature
	add_line( kernel_sign(ver->signature()) );

	// Arguments
	add_line( "(" );
	indent_count++;
	for (auto &node : merge_list) {
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
	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt) {
		if (!scalar[dt].empty()) {
			add_line( scalar_decl(scalar[dt],dt) );
		}
	}

	// Declaring shared memory
	for (auto &pair : shared) {
		add_line( shared_decl(pair.first,pair.second) );
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
		add_line( string("int GC")+n+" = get_group_id("+n+");" );
	}
	for (int n=0; n<N; n++) {
		add_line( string("int GN")+n+" = get_num_groups("+n+");" );
	}
	add_line( "" );

	//// Tagged code sections ////

	for (auto tag : tag_list)
	{
		dispatch_section(tag); //
	}

	indent_count--;
	add_line( "}" ); // Closes kernel body

	//// Printing ////
	std::cout << "***\n" << full_code << "***" << std::endl;

	return full_code;
}

/*********
   Visit
 *********/

void LoopSkeleton::visit_input(Node *node) {
	return; // nothing to do
}

void LoopSkeleton::visit_output(Node *node) {
	if (prod(node->blocksize()) == 1)
		return; // nothing to output for D0
	add_line( out_var(node) + " = " + var_name(node) + ";" );
}

void LoopSkeleton::visit(LoopCond *node) {
	const int N = node->prev()->numdim().toInt();
	string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
	string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

	add_line( lvar + " = " + lvar + " || " + rvar + ";" );

	reduc_list.push_back( SkelReduc(node,node->prev(),rOR,U8) );
	shared.push_back( std::make_pair(node,prod(ver->groupsize())) );
}

//void LoopSkeleton::visit(LoopHead *node) {
//	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
//}


//void LoopSkeleton::visit(LoopTail *node) {
//	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
//}

void LoopSkeleton::visit(Merge *node) {
	merge_list.push_back(node);
}

void LoopSkeleton::visit(Switch *node) {
	switch_list.push_back(node);
}

} } // namespace map::detail
