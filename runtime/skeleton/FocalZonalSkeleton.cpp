/**
 * @file	FocalZonalSkeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "FocalZonalSkeleton.hpp"
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

FocalZonalSkeleton::FocalZonalSkeleton(Version *ver)
	: Skeleton(ver)
	, mask()
	, conv()
	, func()
	, percent()
	//, halo()
	, reduc()
	, zonal_code()
{
	indent_count = 2;
	level = 0;
}

string FocalZonalSkeleton::generate() {
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

void FocalZonalSkeleton::compact() {
	Skeleton::compact();
	//sort_unique(mask,node_id_less(),node_id_equal());
	sort_unique(conv,node_id_less(),node_id_equal());
	sort_unique(func,node_id_less(),node_id_equal());
	sort_unique(percent,node_id_less(),node_id_equal());
}

string FocalZonalSkeleton::versionCode() {
	//// Variables ////
	const int N = 2;
	const Group *group = ver->task->group();

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
		if (!added_F[dt.get()] && isInputOf(node,group).is(FOCAL)) {
			add_section( defines_focal_type(dt) );
			added_F[dt.get()] = true;
			add_line( "" );
		} else if (!added_L[dt.get()] && isInputOf(node,group).is(LOCAL)) {
			add_section( defines_local_type(dt) );
			added_L[dt.get()] = true;
			add_line( "" );
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
	for (auto &node : ver->task->inputList()) { // keeps the order IN_0, IN_8, ...
		if (tag_hash[node].is(PRE_FOCAL))
			add_line( "TYPE_VAR_LIST(" + node->datatype().ctypeString() + ",IN_" + node->id + ")," );
		else if (tag_hash[node].is(INPUT_OUTPUT))
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
		add_line( string("const int _GS") + n + "," + " // @" );
	}
	for (int n=0; n<N; n++) {
		string comma = (n < N-1) ? "," : "";
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
	
	// Declaring focal shared memory
	for (auto &node : shared) {
		add_line( shared_decl(node,prod(ver->groupsize()+2*1)) );
	}

	// Declaring zonal shared memory
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
		add_line( std::string("int H")+n + " = " + "1" + ";" );
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

	//// Previous to focal core ////
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

	// Adds PRE_FOCAL input-nodes
	for (auto &node : ver->task->inputList()) {  // @ erroneus, needs two PRE_FOCAL to differentiate FOCAL / ZONAL
		if (tag_hash[node].is(PRE_FOCAL)) {
			if (isInputOf(node,group).is(FOCAL)) {
				add_line( var_name(node) + " = " + in_var_focal(node) + ";" );
			} else {
				add_line( var_name(node) + " = " + in_var(node) + ";" );	
			}
		}
	}

	// Adds accumulated 'precore' to 'all'
	full_code += code_hash[{PRE_FOCAL,9}];

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

	//// Focal core ////
	add_line( "// FOCAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

	// Adds accumulated 'core' to 'all'
	full_code += code_hash[{FOCAL_CORE,1}];

	indent_count--;
	add_line( "}" ); // Closes global-if
	add_line( "" );

	//// Posterior to focal core ////
	add_line( "// Posterior to FOCAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+") {" );
	indent_count++;

	// Adds LOCAL_CORE input-nodes
	for (auto &node : ver->task->inputList()) {
		if (tag_hash[node].pos != LOCAL_CORE)
			continue;
		if (is_included(node,shared)) {
			add_line( var_name(node) + " = " + var_name(node,SHARED) + "[" + local_proj_focal_H(N) + "];" );
		} else if (tag_hash[node].is(PRE_FOCAL)) {
			add_line( var_name(node) + " = IN_" + node->id + "[" + global_proj(N) + "];" );
		} else {
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
	add_line( "" );

	//// Previous to zonal core ////
	add_line( "// Previous to ZONAL core\n" );

	// Global-if
	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" );
	indent_count++;

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

	//// Zonal Core ////
	add_line( "// Zonal core\n" );

	// Zonal-loop
	add_line( "for (int i="+group_size_prod(N)+"/2; i>0; i/=2) {" );
	indent_count++;
	add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );
	add_line( "if ("+local_proj_zonal(N)+" < i)" );
	add_line( "{" );
	indent_count++;
	
	// Adds accumulated 'core' to 'all'
	full_code += zonal_code;

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
		string ovar = string("OUT_") + node->id + "+idx_" + node->id;
		add_line( atomic + "( " + ovar + " , " + var + ");" );
	}

	indent_count--;
	add_line( "}" ); // Closes write-if
	add_line( "" );

	indent_count--;
	add_line( "}" ); // Closes kernel body

	//// Printing ////
	std::cout << "***\n" << full_code << "***" << std::endl;

	return full_code;
}

/*********
   Visit
 *********/

void FocalZonalSkeleton::visit(Neighbor *node) {
	// Adds Neighbor code
	{
		const int N = node->numdim().toInt();
		Coord nbh = node->coord();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_nbh(N,nbh) + "]";

		add_line( var + " = " + svar + ";" );
	}

	shared.push_back(node->prev());
}

void FocalZonalSkeleton::visit(Convolution *node) {
	// Adds convolution code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";
		string mvar = node->mask().datatype().toString() + "L_" + std::to_string(node->id);

		add_line( var + " = 0;" );

		for (int n=N-1; n>=0; n--) {
			int h = 1;
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

	shared.push_back(node->prev());
	mask.push_back( std::make_pair(node->mask(),node->id) );
}

void FocalZonalSkeleton::visit(FocalFunc *node) {
	// Adds focal code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";

		add_line( var + " = " + node->type.neutralString(node->datatype()) + ";" );

		for (int n=N-1; n>=0; n--) {
			int h = 1;
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

	shared.push_back(node->prev());
	func.push_back(node);
}

void FocalZonalSkeleton::visit(FocalPercent *node) {
	// Adds FocalPercent code
	{
		const int N = node->numdim().toInt();
		string var = var_name(node);
		string pvar = var_name(node->prev());
		string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N) + "]";

		add_line( var + " = 0;" );

		for (int n=N-1; n>=0; n--) {
			int h = 1;
			string i = string("i") + n;
			add_line( "for (int "+i+"=-"+h+"; "+i+"<="+h+"; "+i+"++) {" );
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

void FocalZonalSkeleton::visit(ZonalReduc *node) {
	// Adds zonal code
	{
		const int N = node->prev()->numdim().toInt();
		string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

		if (node->type.isOperator())
			zonal_code += "\t\t\t" + lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";\n";
		else if (node->type.isFunction())
			zonal_code += "\t\t\t" + lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");\n";
		else 
			assert(0);
	}

	reduc.push_back(node);
}

} } // namespace map::detail
