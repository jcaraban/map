/**
 * @file	RadialSection.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "RadialSection.hpp"
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

string RadialSection::section() {
	string str;
	str += format( "// Radial section" );
	str += format( "{" );
	indent_count++;
	str += indented(code);
	indent_count--;
	str += format( "}" );
	str += format( "" );
	return str;
}

/*********
   Visit
 *********/

void RadialSection::visit(RadialScan *node) {
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

	radia.push_back(node);
	shared.push_back( std::make_pair(node,prod(skel->ver->groupsize())+1) );
}

} } // namespace map::detail
