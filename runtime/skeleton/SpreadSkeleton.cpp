/**
 * @file	SpreadSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

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

/***************
   Constructor
 ***************/

SpreadSkeleton::SpreadSkeleton(Version *ver)
	: Skeleton(ver)
{ }

string SpreadSkeleton::generate() {
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

std::string SpreadSkeleton::versionCode() {
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
	add_section( defines_spread() );
	add_section( defines_spread_type(spread[0]->datatype()) );
	if (spread[0]->datatype() != spread[0]->dir()->datatype())
		add_section( defines_spread_type(spread[0]->dir()->datatype()) );
	add_line( "" );
	
	// Signature
	add_line( kernel_sign(ver->signature()) );

	// Arguments
	add_line( "(" );
	indent_count++;
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].pos == PRE_SPREAD)
			add_line( string("TYPE_VAR_LIST(IN_") + node->id + "," + node->datatype().ctypeString() + ")," );
		else if (tag_hash[node].pos == LOCAL_POS)
			add_line( in_arg(node) );
	}
	for (auto &node : ver->task->outputList()) {
		if (node->pattern().is(SPREAD) || node == spread[0]->spread()) // @
			add_line( string("TYPE_VAR_LIST(OUT_") + node->id + "," + node->datatype().ctypeString() + ")," );
		else
			add_line( out_arg(node) );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BS") + n + "," );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int BC") + n + "," );
	}
	for (int n=0; n<N; n++) {
		add_line( string("const int GS") + n + "," );
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

	// Declaring Spread shared memory
	for (auto &node : shared) {
		if (node == spread[0]->stable())
			add_line( shared_decl(node,prod(ver->groupsize())) );
		else
			add_line( shared_decl(node,prod(ver->groupsize()+2)) );
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
		add_line( std::string("int H")+n + " = 1;" );
	}

	add_line( "" );

	//// Previous to core ////
	add_line( "// Previous to Spread core\n" );

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

	// Adds PRE_SPREAD input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].pos == PRE_SPREAD) {
			add_line( var_name(node) + " = " + in_var_focal(node) + ";" );
		}
	}
	//add_line( var_name(spread[0]) + " = " + in_var_spread(spread[0]) + ";" );
	//add_line( var_name(spread[0]->spread()) + " = " + in_var_spread(spread[0]->spread()) + ";" );

	// Adds accumulated 'precore' to 'all'
	full_code += code_hash[{PRE_SPREAD,9}];

	// Filling spread shared memory
	for (auto &node : shared) {
		if (node == spread[0]->stable())
		{
			string svar = var_name(node,SHARED) + "[" + local_proj_focal(N) + "]";
			string var = ReductionType(rOR).neutralString(U16);
			add_line( svar + " = " + var + ";" );	
		}
		else
		{
			string svar = var_name(node,SHARED) + "[" + local_proj_focal(N) + "]";
			string var = var_name(node);
			add_line( svar + " = " + var + ";" );
		}
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
	add_line( "// SPREAD core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

	// Adds accumulated 'core' to 'all'
	full_code += code_hash[{SPREAD_POS,1}];

	indent_count--;
	add_line( "}" ); // Closes global-if
	add_line( "" );

	add_line( "if ("+global_cond(N)+")" ); // Global-if
	add_line( "{" );
	indent_count++;

	// Spread output
	for (auto &node : ver->task->outputList()) {
		if (node == spread[0]->buffer()) {
			add_line( out_var(node) + ";" );
		}
	}

	indent_count--;
	add_line( "}" ); // Closes global-if

	// Stable-if
	add_line( "if ("+local_cond_zonal(N)+")" );
	add_line( "{" );
	indent_count++;

	// Stable output
	string svar = var_name(spread[0]->stable(),SHARED) + "["+local_proj_zonal(N)+"]";
	string osvar = string("OUT_") + spread[0]->stable()->id + "["+group_proj(N)+"]";
	add_line( osvar + " = " + svar + ";" );

	indent_count--;
	add_line( "}" ); // Closes stable-if
	add_line( "" );

	//// Posterior to core ////
	add_line( "// Posterior to SPREAD core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

	// Adds LOCAL_POS input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].pos != LOCAL_POS)
			continue;
		if (is_included(node,shared)) {
			add_line( var_name(node) + " = " + var_name(node,SHARED) + "[" + local_proj_focal_H(N) + "];" );
		} else if (tag_hash[node].pos == PRE_SPREAD) {
			add_line( var_name(node) + " = IN_" + node->id + "_11[" + global_proj(N) + "];" );
		} else {
			add_line( var_name(node) + " = " + in_var(node) + ";" );
		}
	}

	// Adds accumulated 'poscode' to 'all'
	full_code += code_hash[{LOCAL_POS,1}];

	// Adds LOCAL_POS output-nodes
	for (auto &node : ver->task->outputList()) {
		if (tag_hash[node].pos == LOCAL_POS && node->pattern().isNot(SPREAD)) {
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

void SpreadSkeleton::visit(SpreadScan *node) {

	scalar[ node->dir()->datatype().get() ].push_back(node->dir()->id);
	scalar[ node->spread()->datatype().get() ].push_back(node->spread()->id);
	scalar[ node->buffer()->datatype().get() ].push_back(node->buffer()->id);
	scalar[ node->stable()->datatype().get() ].push_back(node->stable()->id);
	//in[node_pos].push_back(node->dir());
	//out[node_pos].push_back(node->spread());
	//out[node_pos].push_back(node->buffer());
	//out[node_pos].push_back(node->stable());

	// Adds Spread code
	{
		const int N = node->numdim().toInt();
		string scan = in_var_spread(node);
		string dir = var_name(node->dir(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";
		string spread = in_var_spread(node->spread());
		string buffer = var_name(node->buffer());
		string stable = var_name(node->stable(),SHARED) + "[" + local_proj_zonal(N) + "]";
		string neutral = node->type.neutralString( node->datatype() );
		string changed = std::string("changed_") + node->stable()->id;

		add_line( buffer + " = " + neutral + ";" );

		for (int n=N-1; n>=0; n--) {
			int h = 1;
			string i = string("i") + n;
			add_line( "for (int "+i+"=-1; "+i+"<=1; "+i+"++) {" );
			indent_count++;
		}

		add_line( "if (" + zero_cond_spread(N) + ") continue;" );
		add_line( "if (" + dir + " != DIRCOD()) continue;" );
		add_line( "if (" + spread + " == " + neutral + ") continue;" );
		add_line( "" );

		add_line( scan + " += " + spread + ";" );
		add_line( buffer + " += " + spread + ";" );
		add_line( spread + " = " + neutral + ";" );

		for (int n=N-1; n>=0; n--) {
			indent_count--;
			add_line( "}" );
		}
		add_line( "" );

		add_line( "if (" + buffer + " > " + neutral + ") {" );
		indent_count++;
		add_line( "bool _c0  = (bc0 == 0);" );
		add_line( "bool  c0_ = (bc0 == BS0-1);" );
		add_line( "bool  c0  = (!_c0 && !c0_);" );
		add_line( "bool _c1  = (bc1 == 0);" );
		add_line( "bool  c1_ = (bc1 == BS1-1);" );
		add_line( "bool  c1  = (!_c1 && !c1_);" );
		add_line( var_name(node->stable()) + " = " );
		indent_count++;
		add_line( "( c0  &&  c1 ) * 0x01 |" );
		add_line( "( c0_ &&  c1 ) * 0x02 |" );
		add_line( "( c0_ &&  c1_) * 0x04 |" );
		add_line( "( c0  &&  c1_) * 0x08 |" );
		add_line( "(_c0  &&  c1_) * 0x10 |" );
		add_line( "(_c0  &&  c1 ) * 0x20 |" );
		add_line( "(_c0  && _c1 ) * 0x40 |" );
		add_line( "( c0  && _c1 ) * 0x80 |" );
		add_line( "( c0_ && _c1 ) * 0x100;" );
		indent_count--;
		indent_count--;
		add_line( "} else {" );
		indent_count++;
		add_line( var_name(node->stable()) + " = " + ReductionType(rOR).neutralString(U16) + ";" );
		indent_count--;
		add_line( "}" ); // Closes changed-if
		add_line( stable + " = " + var_name(node->stable()) + ";" );

		indent_count--;
		add_line( "}" ); // Closes global-if
		add_line( "" );

		add_line( "// Reduccion the stable value\n" );

		// Stable-loop
		add_line( "for (int i="+group_size_prod(N)+"/2; i>0; i/=2) {" );
		indent_count++;
		add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );
		add_line( "if ("+local_proj_zonal(N)+" < i)" );
		add_line( "{" );
		indent_count++;

		// ... continue ...
		string lvar = var_name(node->stable(),SHARED) + "[" + local_proj_zonal(N) + "]";
		string rvar = var_name(node->stable(),SHARED) + "[" + local_proj_zonal(N) + " + i]";
		add_line( lvar + " = " + lvar + " " + ReductionType(rOR).code() + " " + rvar + ";" );

		indent_count--;
		add_line( "}" ); // Closes if
		indent_count--;

	}

	//shared.push_back(node);
	shared.push_back(node->dir());
	//shared.push_back(node->spread());
	shared.push_back(node->stable());

	spread.push_back(node);	
}

} } // namespace map::detail
