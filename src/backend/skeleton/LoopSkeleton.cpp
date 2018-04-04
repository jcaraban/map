/**
 * @file	LoopSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LoopSkeleton.hpp"
#include "../util.hpp"
#include "../Version.hpp"
#include "../Task.hpp"
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
	
	// @ Removes input sections
	int i = 0;
	while (i < section_list.size()) {
		Section *sect = section_list[i++];
		if (sect->is(INPUT)) {
			i--;
			for (auto node : sect->nodeList())
				remove_value(sect,section_list_of[node]);
			section_list.erase(section_list.begin() + i);
		}
	}
	
	fill(); // fill structures
	compact(); // compact structures

	return generateCode();
}

std::string LoopSkeleton::generateCode() {
	//// Variables ////
	const int N = ver->task->numdim().toInt();
	Pattern pattern = ver->task->cluster()->pattern();
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
	bool out_types[N_DATATYPE] = {};

	auto any_true = [](bool array[], int num){
		return std::any_of(array,array+num,[](bool b) { return b; });
	};

	for (auto node : ver->task->inputList()) {
		DataType dt = node->datatype();
		local_types[dt.get()] = true;
	}
	for (auto node : ver->task->outputList()) {
		DataType dt = node->datatype();
		out_types[dt.get()] = true;
	}

	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		if (local_types[dt])
			add_section( defines_local_type(dt) );
	if (any_true(local_types,N_DATATYPE))
		add_line( "" );

	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		if (out_types[dt])
			add_section( defines_output_type(dt) );
	if (any_true(out_types,N_DATATYPE))
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
		if (node->numdim() == D0 && not node->pattern().is(LOOP))
			continue;
		add_line( out_arg(node) );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BS") + n + "," );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BC") + n + "," );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BN") + n + "," );
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

	//// Code sections ////

	for (auto sect : section_list)
	{
		add_section( indented( sect->generateCode() ) );
	}

	indent_count--;
	add_line( "}" ); // Closes kernel body

	//// Printing ////
	std::cout << "***\n" << full_code << "***" << std::endl;

	return full_code;
}

} } // namespace map::detail
