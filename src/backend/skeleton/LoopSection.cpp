/**
 * @file	LoopSection.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LoopSection.hpp"
#include "../Skeleton.hpp"
#include "../Version.hpp"
#include "../Task.hpp"
#include <iostream>
#include <functional>


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
}

//

string LoopSection::section() {
	string str;

	str += format( "// Loop section" );
	str += format( "{" );
	indent_count++;
	/*	
	// Merge inputs
	for (auto merge : merge_list)
			str += format( var_name(merge) + " = " + in_var(merge) + ";" );
	str += format( "" );

	// Switch outputs
	for (auto swit : switch_list)
			str += format( var_name(swit) + " = " + var_name(swit->prev()) + ";" );
	str += format( "" );
	*/
	str += reduc_section();

	// Write-if
	int N = skel->ver->task->numdim().toInt();
	str += format( "if ("+local_cond_zonal(N)+")" );
	str += format( "{" );
	indent_count++;

	// LoopCond output
	for (auto reduc : reduc_list) {
		string atomic = "atomic" + reduc.rt.toString();
		string var = var_name(reduc.node);
		string ovar = string("(global char*)OUT_") + reduc.node->id + "+off_" + reduc.node->id;
		str += format( atomic + "( " + ovar + " , " + var + ");" );
	}

	indent_count--;
	str += format( "}" ); // Closes write-if

	indent_count--;
	str += format( "}" );
	str += format( "" );

	return str;
}

/*********
   Visit
 *********/

void LoopSection::visit(LoopCond *node) {
	const int N = node->prev()->numdim().toInt();
	string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
	string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

	add_line( lvar + " = " + lvar + " || " + rvar + ";" );

	reduc_list.push_back( SkelReduc(node,node->prev(),rOR,U8) );
	shared.push_back( std::make_pair(node,prod(skel->ver->groupsize())) );
}

} } // namespace map::detail
