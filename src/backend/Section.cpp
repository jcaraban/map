/**
 * @file	Skeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: refactor Section into CPU / ZONAL / FOCAL sections?
 * TODO: base the definitions on the in/out spatial reach, instead than in the pattern L/F/Z/R
 */

#include "Section.hpp"
#include "Skeleton.hpp"
#include "skeleton/RadialSection.hpp"
#include "skeleton/LoopSection.hpp"
#include "Version.hpp"
#include "Task.hpp"

#include <iostream>
#include <functional>


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
}

//

Section* Section::Factory(DataSize ext, Pattern pat, Skeleton *skel) {
	assert(pat != NONE_PATTERN);
	Section *sect;

	if ( pat.is(RADIAL) )
	{
		sect = new RadialSection();
	}
	else if ( pat.is(LOOP) )
	{
		sect = new LoopSection();
	}
	else {
		sect = new Section();
	}

	sect->skel = skel;
	sect->ext = ext; // initial extension
	sect->pat = pat; // initial code section position
	sect->dev = skel->ver->dev_type;

	return sect;
}

const NodeList& Section::nodeList() const {
	return node_list;
}

const SectionList& Section::prevList() const {
	return prev_list;
}

const SectionList& Section::nextList() const {
	return next_list;
}

//

bool Section::extendedReach() const {
	return prod(ext) > 1;
}

NumDim Section::numdim() const {
	return DataSize2NumDim(ext);
}

void Section::add(Pattern pat) {
	this->pat += pat;
};

void Section::sub(Pattern pat) {
	this->pat -= pat;
};

bool Section::is(Pattern pat) const {
	return this->pat.is(pat);
};

bool Section::isNot(Pattern pat) const {
	return this->pat.isNot(pat);
};

bool Section::operator==(const Section &sect) const {
	return this->pat == sect.pat && coord_equal()(ext,sect.ext);
};

bool Section::operator>(const Section &sect) const {
	int this_pds = prod(this->ext);
	int sect_pds = prod(sect.ext);
	if (this_pds > sect_pds)
		return 1;
	else if (sect_pds > this_pds)
		return 0;
	else if (this->pat > sect.pat)
		return 1;
	else // sect.pat.pat > this->pat.pat
		return 0;
};

bool Section::operator<(const Section &sect) const {
	int this_pds = prod(this->ext);
	int sect_pds = prod(sect.ext);
	if (this_pds < sect_pds)
		return 1;
	else if (sect_pds < this_pds)
		return 0;
	else if (this->pat < sect.pat)
		return 1;
	else // sect.pat < this->pat
		return 0;
};

std::size_t Section::Hash::operator()(const Section *k) const {
	return k->pat.hash() ^ coord_hash()(k->ext);
}

bool Section::Greater::operator()(const Section *lhs, const Section *rhs) const {
	return *lhs > *rhs;
}

bool Section::Equal::operator()(const Section *lhs, const Section *rhs) const {
	return *lhs == *rhs;
}

//

string Section::indent() {
	string str = "";
	for (int i=0; i<indent_count; i++)
		str += "\t";
	return str;
}

string Section::indented(std::string code) {
	string ind_code;
	int npo2 = 1; // next power of 2
	while(npo2 < code.size())
    	npo2 *= 2;
	ind_code.reserve(npo2);
	string tabs(indent_count,'\t');
	
	ind_code.insert(ind_code.begin(),tabs.begin(),tabs.end());
	for (char charac : code) {
		ind_code.push_back(charac);
		if (charac == '\n')
			ind_code.insert(ind_code.end(),tabs.begin(),tabs.end());
	}
	ind_code.resize(ind_code.size()-tabs.size()); // remove last tabs

	return ind_code;
}

string Section::format(string str) {
	return indent() + str + "\n";
}

void Section::add_line(string str) {
	code += format(str);
}

//

void Section::fill() {
	for (auto node : node_list)
		if (pat == INPUT)
			visit_input(node);
		else if (pat == OUTPUT)
			visit_output(node);
		else
			node->accept(this);
}

string Section::generateCode() {
	return section();
}

string Section::section() {
	if (is(INPUT)) {
		if (dev == DEV_CPU && extendedReach())
			return cpu_input_section();
		else
			return input_section();
	} else {
		if (is(FREE))
			return free_section();
		else if (is(LOCAL))
			return local_section();
		else if (is(FOCAL))
			return focal_section();
		else if (is(ZONAL))
			if (dev == DEV_CPU)
				return cpu_zonal_section();
			else
				return zonal_section();
		else if (is(STATS))
			return stats_section();
		else if (is(MERGE))
			return merge_section();
		else if (is(LOOP))
			assert(0);
		else if (is(SWITCH))
			return switch_section();
		else if (isNot(OUTPUT))
			return code+"\n";
	}
	if (is(OUTPUT)) {
		return output_section();
	}
	assert(0);
}

string Section::input_section() {
	string str;
	str += format( "// Input section" );
	str += format( "{" );
	indent_count++;
	str += indented(code);
	indent_count--;
	str += format( "}" );
	str += format( "" );
	return str;
}

string Section::cpu_input_section() {
	string str;
	str += format( "// Input section" );
	str += format( "{" );
	indent_count++;

	// Outer-part global-if
	str += format( "if (GC0 == 0 || GC0 == GN0-1 || GC1 == 0 || GC1 == GN1-1)" );
	str += format( "{" );
	indent_count++;

	// Adds accumulated 'core' to 'all'
	str += indented(code);

	indent_count--;
	str += format( "}" ); // Closes inner part

	// Inner-part global-if
	str += format( "if (GC0 > 0 && GC0 < GN0-1 && GC1 > 0 && GC1 < GN1-1)" );
	str += format( "{" );
	indent_count++;

	// Re-walk the input nodes for the inner-part code
	for (auto node : nodeList()) {
		str += indent() + var_name(node) + " = " + in_var(node) + ";\n";
	}

	indent_count--;
	str += format( "}" ); // Closes outer part

	indent_count--;
	str += format( "}" );
	str += format( "" );

	return str;
}

string Section::output_section() {
	string str;
	str += format( "// Output section" );
	str += format( "{" );
	indent_count++;
	str += indented(code);
	indent_count--;
	str += format( "}" );
	str += format( "" );
	return str;
}

string Section::free_section() {
	string str;
	str += format( "// Free section" );
	str += format( "{" );
	indent_count++;
	str += indented(code);
	indent_count--;
	str += format( "}" );
	str += format( "" );
	return str;
}

string Section::local_section() {
	string str;
	str += format( "// Local section" );
	str += format( "{" );
	indent_count++;
	str += indented(code);
	indent_count--;
	str += format( "}" );
	str += format( "" );
	return str;
}

string Section::focal_section() {
	string str;
	str += format( "// Focal section" );
	str += format( "{" );
	indent_count++;
	str += indented(code);
	indent_count--;
	str += format( "}" );
	str += format( "" );
	return str;
}

string Section::zonal_section() {
	string str;
	str += format( "// Zonal section" );
	str += format( "{" );
	indent_count++;

	str += reduc_section();

	// Write-if
	int N = skel->ver->task->numdim().toInt();
	str += format( "if ("+local_cond_zonal(N)+")" );
	str += format( "{" );
	indent_count++;

	// Zonal output
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

string Section::cpu_zonal_section() {
	string str;
	str += format( "// Zonal section" );
	str += format( "{" );
	indent_count++;

	// @@
	int N = skel->ver->task->numdim().toInt();

	str += format( "if ("+global_cond(N)+")" );
	str += format( "{" ); // Global-if
	indent_count++;

	// Filling shared memory
	for (auto reduc : reduc_list) {
		string svar = var_name(reduc.node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string var = var_name(reduc.prev);
		str += format( svar + " = " + var + ";" );
	}

	indent_count--;
	str += format( "}" );
	str += format( "else" );
	str += format( "{" );
	indent_count++;

	// Filling shared memory, corner cases with neutral element
	for (auto reduc : reduc_list) {
		string svar = var_name(reduc.node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string neutral = reduc.rt.neutralString(reduc.dt);
		str += format( svar + " = " + neutral + ";" );
	}

	// Closes global-if and synchronizes
	indent_count--;
	str += format( "}" );
	str += format( "barrier(CLK_LOCAL_MEM_FENCE);" );
	str += format( "" );

	str += format( "if ("+local_cond_zonal(N)+")" );
	str += format( "{" );
	indent_count++;

	for (auto reduc : reduc_list)
		str += format( var_name(reduc.node) + " = " + reduc.rt.neutralString(reduc.dt) + ";" );
	str += format( "" );

	// Opening sequential loop
	for (int n=N-1; n>=0; n--) {
		string off = string("gc") + n;
		string lim = string("GS") + n;
		str += format( "for (int "+off+"=0; "+off+"<"+lim+"; "+off+"++) {" );
		indent_count++;
	}

	// Adds accumulated 'core' to 'all'
	str += indented(code);

	// Closing sequential loop
	for (int n=N-1; n>=0; n--) {
		indent_count--;
		str += format( "}" );
	}

	indent_count--;
	str += format( "}" );
	str += format( "" );

	// @@

	// Write-if
	str += format( "if ("+local_cond_zonal(N)+")" );
	str += format( "{" );
	indent_count++;

	// Zonal output
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

string Section::stats_section() {
	string str;

	str += format( "// Stats section" );
	str += format( "{" );
	indent_count++;
	
	str += reduc_section();

	// Write-if
	int N = skel->ver->task->numdim().toInt();
	str += format( "if ("+local_cond_zonal(N)+")" );
	str += format( "{" );
	indent_count++;

	// Stats output
	for (auto reduc : reduc_list) {
		if (prod(reduc.node->blocksize())==1) // BlockSummary
		{
			string atomic = "atomic" + reduc.rt.toString();
			string var = var_name(reduc.node);
			string ovar = string("(global char*)OUT_") + reduc.node->id + "+off_" + reduc.node->id;
			str += format( atomic + "( " + ovar + " , " + var + ");" );
		}
		else // GroupSummary
		{
			string out_var = string("OUT_") + reduc.node->id + "[" + group_proj(N) + "]";
			str += format( out_var + " = " + var_name(reduc.node) + ";" );
		}
	}

	indent_count--;
	str += format( "}" ); // Closes write-if

	indent_count--;
	str += format( "}" );
	str += format( "" );

	return str;
}

string Section::reduc_section() {
	int N = skel->ver->task->numdim().toInt();
	string str;

	str += format( "if ("+global_cond(N)+")" );
	str += format( "{" ); // Global-if
	indent_count++;
	
	// Filling shared memory
	for (auto reduc : reduc_list) {
		string svar = var_name(reduc.node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string var = var_name(reduc.prev);
		str += format( svar + " = " + var + ";" );
	}

	indent_count--;
	str += format( "}" );
	str += format( "else" );
	str += format( "{" );
	indent_count++;

	// Filling shared memory, corner cases with neutral element
	for (auto reduc : reduc_list) {
		string svar = var_name(reduc.node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string neutral = reduc.rt.neutralString(reduc.dt);
		str += format( svar + " = " + neutral + ";" );
	}

	// Closes global-if and synchronizes
	indent_count--;
	str += format( "}" );
	str += format( "" );

	// Reduction-loop
	str += format( "for (int i="+group_size_prod(N)+"/2; i>0; i/=2) {" );
	indent_count++;
	str += format( "barrier(CLK_LOCAL_MEM_FENCE);" );
	str += format( "if ("+local_proj_zonal(N)+" < i)" );
	str += format( "{" );
	indent_count++;
	
	// Adds accumulated 'core' to 'all'
	str += indented(code);

	indent_count--;
	str += format( "}" ); // Closes if

	indent_count--;
	str += format( "}" ); // Closes for
	str += format( "" );
	str += format( "barrier(CLK_LOCAL_MEM_FENCE);" );

	// Final results to variables
	for (auto reduc : reduc_list) {
		str += format( var_name(reduc.node) + " = " + var_name(reduc.node,SHARED) + "[0];" );
	}

	str += format( "" );

	return str;
}

string Section::merge_section() {
	string str;
	str += format( "// Merge section" );
	str += format( "{" );
	indent_count++;
	str += indented(code);
	indent_count--;
	str += format( "}" );
	str += format( "" );
	return str;
}

string Section::switch_section() {
	string str;
	str += format( "// Switch section" );
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

void Section::visit_input(Node *node) {
	// 
	if (extendedReach())
		add_line( var_name(node) + " = " + in_var_focal(node) + ";" );
	else
		add_line( var_name(node) + " = " + in_var(node) + ";" );
}

void Section::visit_output(Node *node) {
	if (prod(node->blocksize()) == 1)
		return; // nothing to output for D0
	add_line( out_var(node) + ";" );
}

void Section::visit(Constant *node) {
	add_line( var_name(node) + " = " + node->cnst.code() + ";" );
}

void Section::visit(Empty *node) {
	//assert(0);
}

void Section::visit(Index *node) {
	string var = var_name(node);
	if (node->dim == D1)
		add_line( var + " = BC0*BS0+bc0;" );
	else if (node->dim == D2)
		add_line( var + " = BC1*BS1+bc1;" );
	else if (node->dim == D3)
		add_line( var + " = BC2*BS2+bc2;" );
	else
		assert(0);
}

void Section::visit(Identity *node) {
	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
}

void Section::visit(Rand *node) {
	std::array<std::string,N_DATATYPE> max_vec = {"/0","UCHAR_MAX","USHRT_MAX","/0","UINT_MAX","/0","/0","/0","ULONG_MAX"};
	int N = node->numdim().toInt();
	DataType type = node->datatype();
	string var = var_name(node);
	string ctr = std::string("ctr_") + node->id;
	string key = std::string("key_") + node->id;
	string bit = (type.sizeOf() > 4) ? "64" : "32";
	string ut = type.toUnsigned().ctypeString();
	string max_str = max_vec[ type.sizeOf() ];

	add_line( "philox2x"+bit+"_ctr_t ctr_"+std::to_string(node->id)+" = {{"+var_name(node->seed())+"}};" );
	add_line( "philox2x"+bit+"_key_t key_"+std::to_string(node->id)+" = {{"+total_proj(N)+"}};" );

	add_line( ctr + " = philox2x"+bit+"("+ctr+","+key+");" );
	if (type.isUnsigned() || type.isSigned()) {
		add_line( var + " = *("+type.ctypeString()+"*)& " + ctr +";" );
	} else { // isFloating()
		add_line( var + " = *("+ut+"*)& " + ctr +" / ("+type.ctypeString()+")"+max_str+";" );
	}

	include.push_back("<Random123/philox.h>");
	//rand.push_back(node);
}

void Section::visit(Cast *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());
	add_line( var + " = " + "("+node->type.ctypeString()+") " + pvar+ + ";" );
}

void Section::visit(Unary *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());

	if (node->type.isOperator())
		add_line( var + " = " + node->type.code() + pvar+ + ";" );
	else if (node->type.isFunction())
		add_line( var + " = " + node->type.code()+"(" + pvar+")" + ";" );
	else 
		assert(0);
}

void Section::visit(Binary *node) {
	string var = var_name(node);
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());

	if (node->type.isOperator())
		add_line( var + " = " + lvar + " "+node->type.code()+" " + rvar + ";" );
	else if (node->type.isFunction())
		add_line( var + " = " + node->type.code()+"(" + lvar+"," + rvar+")" + ";" );
	else 
		assert(0);
}

void Section::visit(Conditional *node) {
	string var = var_name(node);
	string cvar = var_name(node->cond());
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());
	add_line( var + " = (" + cvar + ") ? " + lvar + " : " + rvar + ";" );
}

void Section::visit(Diversity *node) {
	DataType in_type = node->prev(0)->datatype(); // Type of the inputs
	string type_str = in_type.ctypeString();
	int n_arg = node->prevList().size();
	
	string var = var_name(node);
	string elemsa = string("elemSA_") + node->id;
	string cntsa = string("countSA_") + node->id;
	string num = string("num_") + node->id;

	add_line( "int " + num + " = 0;" );

	for (int i=0; i<n_arg; i++) {
		string pvar = var_name(node->prev(i));
		add_line( "addElem_"+type_str+"("+pvar+","+elemsa+","+cntsa+",&"+num+");" );
	}

	if (node->type == VARI)
		add_line( var + " = " + num + ";" );
	else if (node->type == MAJO)
		add_line( var + " = " + "majorElem_"+type_str+"("+elemsa+","+cntsa+","+num+")" + ";" );
	else if (node->type == MINO)
		add_line( var + " = " + "minorElem_"+type_str+"("+elemsa+","+cntsa+","+num+")" + ";" );
	else if (node->type == MEAN)
		add_line( var + " = " + "meanElem_"+type_str+"("+elemsa+","+cntsa+","+num+")" + ";" );
	else
		assert(0);

	diver.push_back(node);
}

void Section::visit(Neighbor *node) {
	const int N = node->numdim().toInt();
	Coord nbh = node->coord();
	string var = var_name(node);
	string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_nbh(N,nbh,node->id) + "]";

	for (int n=0; n<N; n++)
		add_line( string("int ") + "H" + n + "_" + node->id + " = " + node->inputReach().datasize()[n]/2 + ";" );

	add_line( var + " = " + svar + ";" );

	// @
	auto prev_ext = node->inputReach().datasize();
	ext_shared.push_back({node->prev(),prev_ext});
}

void Section::visit(BoundedNeighbor *node) {
	assert(0);
}

void Section::visit(SpreadNeighbor *node) {
	assert(0);
}

void Section::visit(Convolution *node) {
	const int N = node->numdim().toInt();

	string var = var_name(node);
	string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N,node->id) + "]";
	string mvar = node->mask().datatype().toString() + "L_" + std::to_string(node->id);
	string hvar[N];
	for (int n=0; n<N; n++)
		hvar[n] = string("H") + n + "_" + node->id;

	for (int n=0; n<N; n++)
		add_line( string("int ") + hvar[n] + " = " + node->mask().datasize()[n]/2 + ";" );
	add_line( var + " = 0;" );

	for (int n=N-1; n>=0; n--) {
		string i = string("i") + n;
		add_line( "for (int "+i+"=-"+hvar[n]+"; "+i+"<="+hvar[n]+"; "+i+"++) {" );
		indent_count++;
	}

	for (int n=N-1; n>=0; n--)
		mvar += string("[")+"i"+n+"+"+hvar[n]+"]";

	add_line( var + " += " + svar + " * " + mvar + ";" );

	for (int n=N-1; n>=0; n--) {
		indent_count--;
		add_line( "}" );
	}
	
	mask.push_back( std::make_pair(node->mask(),node->id) );
	// @
	auto prev_ext = node->inputReach().datasize();
	ext_shared.push_back({node->prev(),prev_ext});
}

void Section::visit(FocalFunc *node) {
	const int N = node->numdim().toInt();

	string var = var_name(node);
	string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_Hi(N,node->id) + "]";
	string mvar = node->mask().datatype().toString() + "L_" + std::to_string(node->id);
	string hvar[N];
	for (int n=0; n<N; n++)
		hvar[n] = string("H") + n + "_" + node->id;

	for (int n=0; n<N; n++)
		add_line( string("int ") + hvar[n] + " = " + node->mask().datasize()[n]/2 + ";" );
	add_line( var + " = " + node->type.neutralString(node->datatype()) + ";" );

	for (int n=N-1; n>=0; n--) {
		string i = string("i") + n;
		add_line( "for (int "+i+"=-"+hvar[n]+"; "+i+"<="+hvar[n]+"; "+i+"++) {" );
		indent_count++;
	}

	for (int n=N-1; n>=0; n--)
		mvar += string("[")+"i"+n+"+"+hvar[n]+"]";

	add_line( "if (" + mvar + ") {" );
	indent_count++;

	if (node->type.isOperator())
		add_line( var + " = " + var + " "+node->type.code()+" " + svar + ";" );
	else if (node->type.isFunction())
		add_line( var + " = " + node->type.code()+"(" + var+"," + svar+")" + ";" );
	else 
		assert(0);

	for (int n=N-1+1; n>=0; n--) {
		indent_count--;
		add_line( "}" );
	}
	
	mask.push_back( std::make_pair(node->mask(),node->id) );
	// @
	auto prev_ext = node->inputReach().datasize();
	ext_shared.push_back({node->prev(),prev_ext});
}

void Section::visit(FocalPercent *node) {
	assert(0);
}

void Section::visit(ZonalReduc *node) {
	if (dev == DEV_GPU)
	{
		const int N = node->prev()->numdim().toInt();
		string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

		if (node->type.isOperator())
			add_line( lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";" );
		else if (node->type.isFunction())
			add_line( lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");" );
		else 
			assert(0);

		reduc_list.push_back( SkelReduc(node,node->prev(),node->type,node->datatype()) );
		shared.push_back( std::make_pair(node,prod(skel->ver->groupsize())) );
	}
	else if (dev == DEV_CPU)
	{
		const int N = node->prev()->numdim().toInt();
		string lvar = var_name(node);
		string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";

		if (node->type.isOperator())
			add_line( lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";" );
		else if (node->type.isFunction())
			add_line( lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");" );
		else 
			assert(0);

		reduc_list.push_back( SkelReduc(node,node->prev(),node->type,node->datatype()) );
		shared.push_back( std::make_pair(node,prod(skel->ver->groupsize())) );
	}
}

void Section::visit(RadialScan *node) {
	assert(0);
}

void Section::visit(SpreadScan *node) {
	assert(0);
}

void Section::visit(LoopCond *node) {
	assert(0);
}

void Section::visit(LoopHead *node) {
	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
}


void Section::visit(LoopTail *node) {
	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
}

void Section::visit(Merge *node) {
	add_line( var_name(node) + " = " + in_var(node) + ";" );
	merge_list.push_back(node);
}

void Section::visit(Switch *node) {
	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
	switch_list.push_back(node);
}


void Section::visit(LhsAccess *node) {
	string var = var_name(node);
	Coord coord = node->cell_coord;
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());
	int N = node->numdim().toInt();
	add_line( var + " = (" + equal_coord_cond(N,coord) + ") ? " + rvar + " : " + lvar + ";" );
}

void Section::visit(Access *node) {
	string var = var_name(node);
	Coord coord = node->cell_coord;
	string pvar = var_name(node->prev());
	int N = node->prev()->numdim().toInt();

	add_line( "if (" + equal_coord_cond(N,coord) + ") " + var + " = " + pvar + ";" );
}

void Section::visit(Read *node) {
	assert(0);
}

void Section::visit(Write *node) {
	assert(0);
}

void Section::visit(Scalar *node) {
	assert(0);
}

void Section::visit(Temporal *node) {
	// nothing to do
}

void Section::visit(Checkpoint *node) {
	assert(0);
}

void Section::visit(Barrier *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}

void Section::visit(Summary *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}

void Section::visit(DataSummary *node) {
	const int N = node->prev()->numdim().toInt();
	string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
	string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

	if (node->type.isOperator())
		add_line( lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";" );
	else if (node->type.isFunction())
		add_line( lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");" );
	else 
		assert(0);

	reduc_list.push_back( SkelReduc(node,node->prev(),node->type,node->datatype()) );
	shared.push_back( std::make_pair(node,prod(skel->ver->groupsize())) );
}

void Section::visit(BlockSummary *node) {
	const int N = node->prev()->numdim().toInt();
	string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
	string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

	if (node->type.isOperator())
		add_line( lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";" );
	else if (node->type.isFunction())
		add_line( lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");" );
	else 
		assert(0);

	reduc_list.push_back( SkelReduc(node,node->prev(),node->type,node->datatype()) );
	shared.push_back( std::make_pair(node,prod(skel->ver->groupsize())) );
}

void Section::visit(GroupSummary *node) {
	const int N = node->prev()->numdim().toInt();
	string lvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + "]";
	string rvar = var_name(node,SHARED) + "[" + local_proj_zonal(N) + " + i]";

	if (node->type.isOperator())
		add_line( lvar + " = " + lvar + " " + node->type.code() + " " + rvar + ";" );
	else if (node->type.isFunction())
		add_line( lvar + " = " + node->type.code() + "(" + lvar + ", " + rvar + ");" );
	else 
		assert(0);

	reduc_list.push_back( SkelReduc(node,node->prev(),node->type,node->datatype()) );
	shared.push_back( std::make_pair(node,prod(skel->ver->groupsize())) );
}

} } // namespace map::detail
