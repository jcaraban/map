/**
 * @file	RadialSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "RadialSkeleton.hpp"
#include "../util.hpp"
#include "../Version.hpp"
#include "../Task.hpp"
#include <stack>
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
	//, radia()
{
	indent_count = 4;
}

string RadialSkeleton::generate() {
	tag(); // tag the nodes
	fill(); // fill structures
	compact(); // compact structures

	// @
	for (auto sect : section_list) {
		radia.insert(radia.end(),sect->radia.begin(),sect->radia.end());
	}

	Direction fst, snd;
	RadialCase rcase = str2radia(ver->detail);
	radia2dir(rcase,fst,snd);

	return generateCode(rcase,fst,snd);
}

/***********
   Methods
 ***********/

string RadialSkeleton::generateCode(RadialCase rcase, Direction fst, Direction snd) {
	//// Variables ////
	const int N = 2;
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
	add_section( defines_radial() );
	add_line( "" );

	bool local_types[N_DATATYPE] = {};
	bool radial_types[N_DATATYPE] = {};
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
		if (node->pattern().is(RADIAL))
			radial_types[dt.get()] = true;
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
	
	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		if (radial_types[dt])
			add_section( defines_radial_type(dt) );
	if (any_true(radial_types,N_DATATYPE))
		add_line( "" );

	//// Find pre-radial nodes // @ no fusion, no need for pre-radial 

	std::unordered_map<Section*,bool,Section::Hash> pre_radial;
	std::stack<Section*> stack;

	for (auto sect : section_list)
		if (sect->is(RADIAL))
			stack.push(sect);

	while (not stack.empty()) {
		auto sect = stack.top();
		stack.pop();
		for (auto prev : sect->prevList()) {
			pre_radial[prev] = true;
			stack.push(prev);
		}
	}

	//// Header ////
	
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
		else //if (section_hash[node].is(INPUT))
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

	// Declaring shared memory
	for (auto &pair : shared) {
		add_line( shared_decl(pair.first,pair.second) );
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

	// Adds accumulated 'precore' to 'all'
	for (auto sect : section_list)
		if (pre_radial[sect])
			add_section( indented( sect->generateCode() ) );

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
	for (auto sect : section_list)
		if (sect->is(RADIAL))
			add_section( indented(sect->code) );

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

	// Adds accumulated 'poscode' to 'all'
	for (auto sect : section_list)
		if (not pre_radial[sect] && not sect->is(RADIAL))
			add_section( indented(sect->code) );

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

} } // namespace map::detail