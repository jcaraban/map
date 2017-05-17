/**
 * @file	Skeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: base the definitions on the in/out spatial reach, instead than in the pattern L/F/Z/R
 */

#include "Skeleton.hpp"
//#include "CpuFocalSkeleton.hpp"
#include "RadialSkeleton.hpp"
#include "LoopSkeleton.hpp"
#include "../Version.hpp"
#include "../task/Task.hpp"
#include <iostream>
#include <functional>


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
}

/***********
   SkelTag
 ***********/

bool SkelTag::extendedReach() const {
	return prod(ext) > 1;
}

void SkelTag::add(Pattern pat) {
	this->pat += pat;
};

void SkelTag::sub(Pattern pat) {
	this->pat -= pat;
};

bool SkelTag::is(Pattern pat) const {
	return this->pat.is(pat);
};

bool SkelTag::isNot(Pattern pat) const {
	return this->pat.isNot(pat);
};

bool SkelTag::operator==(const SkelTag &tag) const {
	return this->pat == tag.pat && all(this->ext == tag.ext);
};

bool SkelTag::operator<(const SkelTag &tag) const {
	int this_pds = prod(this->ext);
	int tag_pds = prod(tag.ext);
	if (this_pds > tag_pds)
		return 1;
	else if (tag_pds > this_pds)
		return 0;
	else if (this->pat.pat < tag.pat.pat)
		return 1;
	else // tag.pat.pat < this->pat.pat
		return 0;
};

bool SkelTag::operator>(const SkelTag &tag) const {
	int this_pds = prod(this->ext);
	int tag_pds = prod(tag.ext);
	if (this_pds < tag_pds)
		return 1;
	else if (tag_pds < this_pds)
		return 0;
	else if (this->pat.pat > tag.pat.pat)
		return 1;
	else // tag.pat > this->pat
		return 0;
};

std::size_t SkelTag::Hash::operator()(const SkelTag& k) const {
	return std::hash<int>()(k.pat.pat) ^  coord_hash()(k.ext);
}

/***************
   Constructor
 ***************/

Skeleton* Skeleton::Factory(Version *ver) {
	Pattern pat = ver->task->pattern();

	if ( pat.is(RADIAL) )
	{
		return new RadialSkeleton(ver);
	}
	else if ( pat.is(FOCAL) && ver->deviceType() == DEV_CPU )
	{
		assert(0); //return new CpuFocalSkeleton(ver);
	}
	else if ( pat.is(LOOP) )
	{
		return new LoopSkeleton(ver);
	}
	else {
		return new Skeleton(ver);
	}
}

Skeleton::Skeleton(Version *ver)
	: ver(ver)
	, indent_count(-1)
{ }

string Skeleton::generate() {
	tag(); // tag the nodes
	fill(); // fill structures
	compact(); // compact structures

	return versionCode();
}

/***********
   Methods
 ***********/

void Skeleton::tag() {
	// Puts in + body + out nodes into 'all_list', in order
	auto body_out = full_unique_join(ver->task->nodeList(),ver->task->outputList());
	auto all_list = full_join(ver->task->inputList(),body_out);

	// Walks nodes backward and accumulate their 'tag = { Pattern , prod_data_size }'
	for (auto it=all_list.rbegin(); it!=all_list.rend(); it++) {
		Node *node = *it;
		SkelTag tag;

		tag.ext = node->inputReach().datasize(); // accumulated extension
		tag.pat = NONE_PATTERN; // code section position

		if (is_included(node,ver->task->inputList())) {
			tag.add(INPUT);
		} else {
			tag.add(node->pattern());
		}
		if (is_included(node,ver->task->outputList())) {
			tag.add(OUTPUT);
		}
		assert(tag.pat != NONE_PATTERN);

		// Splits OTHER + OUTPUT into two sections
		std::vector<SkelTag> tag_vec;
		if (tag.is(OUTPUT) && tag.pat != OUTPUT) {
			SkelTag other_tag = tag, out_tag = tag;
			other_tag.sub(OUTPUT);
			out_tag.pat = OUTPUT;

			tag_vec.push_back(other_tag);
			tag_vec.push_back(out_tag);

			next_of[other_tag].insert(out_tag);
			prev_of[out_tag].insert(other_tag);
		} else {
			// Normal case w/o OUTPUT
			tag_vec.push_back(tag);
		}

		// For every split tag, accumulates 'partial spatial reaches'
		for (auto tag : tag_vec) {
			auto next_inside = inner_join(node->nextList(),body_out);
			for (auto next : next_inside) {
				for (auto next_tag : tag_hash[next]) {
					SkelTag partial = tag;

					auto next_ext = next->inputReach().datasize();
					partial.ext = max(next_ext,next_tag.ext);

					if (tag_set.find(partial) == tag_set.end()) {
						tag_set.insert(partial);
						tag_list.push_back(partial);
						next_of[partial].insert(next_tag);
						prev_of[next_tag].insert(partial);
					}
					if (not is_included(node,node_list_of[partial])) {
						tag_hash[node].push_back(partial);
						node_list_of[partial].push_back(node);
					}
				}
			}
			
			if (next_inside.empty()) {
				if (tag_set.find(tag) == tag_set.end()) {
					tag_set.insert(tag);
					tag_list.push_back(tag);
				}
				if (not is_included(node,node_list_of[tag])) {
					tag_hash[node].push_back(tag);
					node_list_of[tag].push_back(node);
				}
			}
		}
	}

	// Topological sort of 'tags', in order of dependencies and tag-order
	tag_list = sort(tag_list);
}

TagList Skeleton::sort(TagList list) {
	assert(not list.empty());
	TagList ordered_list;

	// Walks all 'tags' first to registers their 'prevs'
	for (auto tag : list) {
		int count = prev_of[tag].size();

		if (count == 0) { // ready 'tags' with 'prev = 0' go into the prique
			prique.push(tag);
		} else { // 'tags' with 'prev > 0' store the count until 'prev = 0'
			prev_count.insert({tag,count});
		}
	}

	// (1) 'queue' for the bfs topo-sort, (2) 'priority' for the tag-sort
	while (not prique.empty()) {
		SkelTag tag = prique.top();
		prique.pop();

		ordered_list.push_back(tag);

		for (auto next : next_of[tag]) {
			auto it = prev_count.find(next);
			assert(it != prev_count.end());
			auto &count = it->second;

			count--;
			if (count == 0) {
				prev_count.erase(next);
				prique.push(next);
			}
		}	
	}

	assert(prev_count.empty());
	return ordered_list;
}

void Skeleton::fill() {
	// Puts in + body + out nodes into 'all_list', in order
	auto all_list = full_join(ver->task->inputList(),ver->task->nodeList());
	all_list = full_unique_join(all_list,ver->task->outputList());

	code_hash.clear();
	full_code.clear();
	indent_count = 1;

	// Reserves some decent length for the strings
	const int avg_skel_len = 4096;	
	for (auto tag : tag_list)
		code_hash[tag].reserve(avg_skel_len);

	// Walks input nodes
	for (auto node : ver->task->inputList()) {
		for (auto tag : tag_hash[node]) {
			curr_tag = tag;
			visit_input(node);
		}
	}

	// Walks body nodes
	for (auto node : ver->task->nodeList()) {
		for (auto tag : tag_hash[node]) {
			curr_tag = tag;
			if (not tag.is(OUTPUT))
				node->accept(this);
		}
	}

	// Walks output nodes
	for (auto node : ver->task->outputList()) {
		for (auto tag : tag_hash[node]) {
			curr_tag = tag;
			if (tag.is(OUTPUT))
				visit_output(node);
		}
	}

	// Walks in + body nodes, registering the necessary scalar variables
	for (auto node : full_join(ver->task->inputList(),ver->task->nodeList()))
		scalar[ node->datatype().get() ].push_back(node->id);

	curr_tag = SkelTag(); // { NONE_PATTERN , DataSize() };
	indent_count = -1;
}

void Skeleton::compact() {
	//sort_unique(diver,node_id_less(),node_id_equal());

	sort_unique(ext_shared,node_id_less(),node_id_equal());
	// Transfers nodes to 'shared' structure
	for (auto node : ext_shared) {
		auto reach = ver->task->accuInputReach(node,Coord());
		int shared_size = prod(ver->groupsize() + reach.datasize() / 2 * 2);
		shared.push_back( std::make_pair(node,shared_size) );
	}

	// Compacts 'shared' structure
	/*
	typedef std::pair<Node*,int> pair;
	auto greater_pred = [](pair lhs, pair rhs) { return lhs.first->id > rhs.first->id; };
	auto equal_pred = [](pair lhs, pair rhs) { return lhs.first->id == rhs.first->id; };
	sort_unique(shared,less_pred,equal_pred);
	*/
}

std::string Skeleton::versionCode() {
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
	if (pattern.is(FOCAL)) {
		add_section( defines_focal() );
		add_line( "" );
	}

	bool local_types[N_DATATYPE] = {};
	bool focal_types[N_DATATYPE] = {};
	bool diver_types[N_DATATYPE] = {};
	bool reduc_def[N_DATATYPE][N_REDUCTION] = {};

	auto any_true = [](bool array[], int num){
		return std::any_of(array,array+num,[](bool b) { return b; });
	};

	for (auto node : ver->task->inputList()) {
		DataType dt = node->datatype();
		auto reach = ver->task->accuInputReach(node,Coord());
		bool extended = prod(reach.datasize()) > 1;

		local_types[dt.get()] = true;
		if (extended)
			focal_types[dt.get()] = true;
	}
	for (auto node : diver) {
		DataType dt = node->prev(0)->datatype();
		diver_types[dt.get()] = true;
	}
	for (auto reduc : reduc_list) {
		reduc_def[reduc.dt.get()][reduc.rt.get()] = true;
	}

	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		if (local_types[dt])
			add_section( defines_local_type(dt) );
	if (any_true(local_types,N_DATATYPE))
		add_line( "" );

	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		if (focal_types[dt])
			add_section( defines_focal_type(dt) );
	if (any_true(focal_types,N_DATATYPE))
		add_line( "" );

	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		if (diver_types[dt])
			add_section( defines_diver_type(dt) );
	if (any_true(diver_types,N_DATATYPE))
		add_line( "" );

	for (auto dt=NONE_DATATYPE; dt<N_DATATYPE; ++dt)
		for (auto rt=NONE_REDUCTION; rt<N_REDUCTION; ++rt)
			if (reduc_def[dt][rt])
				add_section( defines_reduc_type(rt,dt) );
	if (any_true(&**reduc_def,N_DATATYPE*N_REDUCTION))
		add_line( "" );

	//// Header ////

	// Signature
	add_line( kernel_sign(ver->signature()) );

	// Arguments
	add_line( "(" );
	indent_count++;
	for (auto &node : ver->task->inputList()) // keeps the order IN_0, IN_8, ...
	{
		auto reach = ver->task->accuInputReach(node,Coord());
		bool extended = prod(reach.datasize()) > 1;
		add_line( in_arg(node,extended) );
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

	// Decaring masks
	for (auto &pair : mask) {
		add_line( mask_decl(pair.first,pair.second) );
	}
	if (not mask.empty()) {
		add_line( "" );
	}

	// Declaring diversity shared memory
	for (auto &node : diver) {
		int n_arg = node->prevList().size(); 
		int size = n_arg * prod(ver->groupsize());
		auto dt = node->prev(0)->datatype();
		add_line( diver_decl(node,n_arg,size,dt) );
	}
	if (!diver.empty()) {
		add_line( "" );
	}

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

string Skeleton::indent() {
	string str = "";
	for (int i=0; i<indent_count; i++)
		str += "\t";
	return str;
}

string Skeleton::indented(std::string code) {
	string ind_code;
	int npo2 = 1; // next power of 2
	while(npo2 < code.size())
    	npo2 *= 2;
	ind_code.reserve(npo2);

	bool new_line = true;
	for (char charac : code) {
		if (charac == '\t' && new_line) {
			for (int i=0; i<indent_count; i++)
				ind_code.push_back('\t');
			new_line = false;
		} else {
			ind_code.push_back(charac);
			if (charac == '\n')
				new_line = true;
		}
	}

	return ind_code;
}

void Skeleton::add_line(string line) {
	if (curr_tag.pat == NONE_PATTERN)
		full_code += indent() + line + "\n";
	else
		code_hash[curr_tag] += indent() + line + "\n";
}

void Skeleton::add_section(string section) {
	full_code += section;
}

void Skeleton::add_include(string file) {
	// Because the OpenCL kernels are composed at runtime and the user's program could execute anywhere,
	// the includes need to be accessible from any possible path. Copying the header files around with the
	// executables sounds stupid. Defining an absolute path like /usr/include/CL/cl.h seems more elegant.
	// The easiest way now is to 1) paste the include into the kernel, 2) cp it to a tmp in the curr dir.

	// TODO
}

void Skeleton::dispatch_section(SkelTag tag) {

	if (tag.extendedReach()) {
		reach_top_section(tag);
	}

	if (tag.is(INPUT)) {
		input_section(tag);
	} else {
		if (tag.is(FREE))
			free_section(tag);
		if (tag.is(LOCAL))
			local_section(tag);
		if (tag.is(FOCAL))
			focal_section(tag);
		if (tag.is(ZONAL))
			zonal_section(tag);
		if (tag.is(STATS))
			stats_section(tag);
		if (tag.is(LOOP))
			loop_section(tag);
	}
	if (tag.is(OUTPUT)) {
		output_section(tag);
	}

	if (tag.extendedReach()) {
		reach_bot_section(tag);
	}
}

void Skeleton::reach_top_section(SkelTag tag) {
	int N = ver->task->numdim().toInt();

	add_line( "// Extended section" );
	add_line( "{" );
	indent_count++;

	// Extension of the halos
	for (int n=0; n<N; n++) {
		add_line( string("int H")+n + " = " + tag.ext[n]/2 + ";" );
	}
	add_line( "" );

	// Needs to loop because there are fewer threads than extended cells
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
}

void Skeleton::reach_bot_section(SkelTag tag) {
	int N = ver->task->numdim().toInt();

	// Filling focal shared memory
	for (auto node : ext_shared) {
		string svar = var_name(node,SHARED) + "[" + local_proj_focal(N) + "]";
		string var = var_name(node);
		add_line( svar + " = " + var + ";" );
	}
	
	// Closes load-loop
	indent_count--;
	add_line( "}" );
	// Synchronizes
	add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );
	// Closes extended section
	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::input_section(SkelTag tag) {
	add_line( "// Input section" );
	if (tag.ext.size() == 0)
		add_line( "// " + tag.pat.toString() + " " + to_string(tag.ext) );
	add_line( "{" );
	indent_count++;
	full_code += indented(code_hash[tag]);
	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::output_section(SkelTag tag) {
	add_line( "// Output section" );
	add_line( "{" );
	indent_count++;
	full_code += indented(code_hash[tag]);
	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::free_section(SkelTag tag) {
	add_line( "// Free section" );
	add_line( "{" );
	indent_count++;
	full_code += indented(code_hash[tag]);
	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::local_section(SkelTag tag) {
	add_line( "// Local section" );
	add_line( "{" );
	indent_count++;
	full_code += indented(code_hash[tag]);
	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::focal_section(SkelTag tag) {
	add_line( "// Focal section" );
	add_line( "{" );
	indent_count++;
	full_code += indented(code_hash[tag]);
	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::zonal_section(SkelTag tag) {
	add_line( "// Zonal section" );
	add_line( "{" );
	indent_count++;

	reduc_section(tag);

	// Write-if
	int N = ver->task->numdim().toInt();
	add_line( "if ("+local_cond_zonal(N)+")" );
	add_line( "{" );
	indent_count++;

	// Zonal output
	for (auto reduc : reduc_list) {
		if (not is_included(reduc.node,node_list_of[tag]))
			continue;
		string atomic = "atomic" + reduc.rt.toString();
		string var = var_name(reduc.node);
		string ovar = string("(global char*)OUT_") + reduc.node->id + "+off_" + reduc.node->id;
		add_line( atomic + "( " + ovar + " , " + var + ");" );
	}

	indent_count--;
	add_line( "}" ); // Closes write-if

	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::stats_section(SkelTag tag) {
	add_line( "// Stats section" );
	add_line( "{" );
	indent_count++;
	
	reduc_section(tag);

	// Write-if
	int N = ver->task->numdim().toInt();
	add_line( "if ("+local_cond_zonal(N)+")" );
	add_line( "{" );
	indent_count++;

	// Stats output
	for (auto reduc : reduc_list) {
		if (not is_included(reduc.node,node_list_of[tag]))
			continue;
		if (prod(reduc.node->blocksize())==1) // BlockSummary
		{
			string atomic = "atomic" + reduc.rt.toString();
			string var = var_name(reduc.node);
			string ovar = string("(global char*)OUT_") + reduc.node->id + "+off_" + reduc.node->id;
			add_line( atomic + "( " + ovar + " , " + var + ");" );
		}
		else // GroupSummary
		{
			string out_var = string("OUT_") + reduc.node->id + "[" + group_proj(N) + "]";
			add_line( out_var + " = " + var_name(reduc.node) + ";" );
		}
	}

	indent_count--;
	add_line( "}" ); // Closes write-if


	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::loop_section(SkelTag tag) {
	add_line( "// Loop section" );
	add_line( "{" );
	indent_count++;
	
	// Merge inputs
	for (auto merge : merge_list)
		add_line( var_name(merge) + " = " + in_var(merge) + ";" );
	add_line( "" );

	reduc_section(tag);

	// Write-if
	int N = ver->task->numdim().toInt();
	add_line( "if ("+local_cond_zonal(N)+")" );
	add_line( "{" );
	indent_count++;

	// LoopCond output
	for (auto reduc : reduc_list) {
		if (not is_included(reduc.node,node_list_of[tag]))
			continue;
		string atomic = "atomic" + reduc.rt.toString();
		string var = var_name(reduc.node);
		string ovar = string("(global char*)OUT_") + reduc.node->id + "+off_" + reduc.node->id;
		add_line( atomic + "( " + ovar + " , " + var + ");" );
	}

	indent_count--;
	add_line( "}" ); // Closes write-if


	indent_count--;
	add_line( "}" );
	add_line( "" );
}

void Skeleton::reduc_section(SkelTag tag) {
	int N = ver->task->numdim().toInt();

	add_line( "if ("+global_cond(N)+")" );
	add_line( "{" ); // Global-if
	indent_count++;
	
	// Filling shared memory
	for (auto reduc : reduc_list) {
		if (not is_included(reduc.node,node_list_of[tag]))
			continue;
		string svar = var_name(reduc.node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string var = var_name(reduc.prev);
		add_line( svar + " = " + var + ";" );
	}

	indent_count--;
	add_line( "}" );
	add_line( "else" );
	add_line( "{" );
	indent_count++;

	// Filling shared memory, corner cases with neutral element
	for (auto reduc : reduc_list) {
		if (not is_included(reduc.node,node_list_of[tag]))
			continue;
		string svar = var_name(reduc.node,SHARED) + "[" + local_proj_zonal(N) + "]";
		string neutral = reduc.rt.neutralString(reduc.dt);
		add_line( svar + " = " + neutral + ";" );
	}

	// Closes global-if and synchronizes
	indent_count--;
	add_line( "}" );
	add_line( "" );

	// Reduction-loop
	add_line( "for (int i="+group_size_prod(N)+"/2; i>0; i/=2) {" );
	indent_count++;
	add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );
	add_line( "if ("+local_proj_zonal(N)+" < i)" );
	add_line( "{" );
	indent_count++;
	
	// Adds accumulated 'core' to 'all'
	full_code += indented(code_hash[tag]);

	indent_count--;
	add_line( "}" ); // Closes if

	indent_count--;
	add_line( "}" ); // Closes for
	add_line( "" );
	add_line( "barrier(CLK_LOCAL_MEM_FENCE);" );

	// Final results to variables
	for (auto reduc : reduc_list) {
		if (not is_included(reduc.node,node_list_of[tag]))
			continue;
		add_line( var_name(reduc.node) + " = " + var_name(reduc.node,SHARED) + "[0];" );
	}

	add_line( "" );
}

/*********
   Visit
 *********/

void Skeleton::visit_input(Node *node) {
	// 
	if (curr_tag.extendedReach())
		add_line( var_name(node) + " = " + in_var_focal(node) + ";" );
	else
		add_line( var_name(node) + " = " + in_var(node) + ";" );
}

void Skeleton::visit_output(Node *node) {
	if (prod(node->blocksize()) == 1)
		return; // nothing to output for D0
	add_line( out_var(node) + " = " + var_name(node) + ";" );
}

void Skeleton::visit(Constant *node) {
	add_line( var_name(node) + " = " + node->cnst.toString() + ";" );
}

void Skeleton::visit(Index *node) {
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

void Skeleton::visit(Identity *node) {
	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
}

void Skeleton::visit(Rand *node) {
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
	rand.push_back(node);
}

void Skeleton::visit(Cast *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());
	add_line( var + " = " + "("+node->type.ctypeString()+") " + pvar+ + ";" );
}

void Skeleton::visit(Unary *node) {
	string var = var_name(node);
	string pvar = var_name(node->prev());

	if (node->type.isOperator())
		add_line( var + " = " + node->type.code() + pvar+ + ";" );
	else if (node->type.isFunction())
		add_line( var + " = " + node->type.code()+"(" + pvar+")" + ";" );
	else 
		assert(0);
}

void Skeleton::visit(Binary *node) {
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

void Skeleton::visit(Conditional *node) {
	string var = var_name(node);
	string cvar = var_name(node->cond());
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());
	add_line( var + " = (" + cvar + ") ? " + lvar + " : " + rvar + ";" );
}

void Skeleton::visit(Diversity *node) {
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

void Skeleton::visit(Neighbor *node) {
	const int N = node->numdim().toInt();
	Coord nbh = node->coord();
	string var = var_name(node);
	string svar = var_name(node->prev(),SHARED) + "[" + local_proj_focal_nbh(N,nbh) + "]";

	add_line( var + " = " + svar + ";" );

	//int shsz = prod(ver->groupsize() + sum(abs(nbh)) * 2);
	//shared.push_back( std::make_pair(node->prev(),shsz) );
	ext_shared.push_back(node->prev());
}

void Skeleton::visit(BoundedNeighbor *node) {
	assert(0);
}

void Skeleton::visit(SpreadNeighbor *node) {
	assert(0);
}

void Skeleton::visit(Convolution *node) {
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
	ext_shared.push_back(node->prev());
}

void Skeleton::visit(FocalFunc *node) {
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
	ext_shared.push_back(node->prev());
}

void Skeleton::visit(FocalPercent *node) {
	assert(0);
}

void Skeleton::visit(ZonalReduc *node) {
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
	shared.push_back( std::make_pair(node,prod(ver->groupsize())) );
}

void Skeleton::visit(RadialScan *node) {
	assert(0);
}

void Skeleton::visit(SpreadScan *node) {
	assert(0);
}

void Skeleton::visit(LoopCond *node) {
	assert(0);
}

void Skeleton::visit(LoopHead *node) {
	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
}


void Skeleton::visit(LoopTail *node) {
	add_line( var_name(node) + " = " + var_name(node->prev()) + ";" );
}

void Skeleton::visit(Merge *node) {
	assert(0);
}

void Skeleton::visit(Switch *node) {
	assert(0);
}

void Skeleton::visit(LhsAccess *node) {
	string var = var_name(node);
	Coord coord = node->cell_coord;
	string lvar = var_name(node->left());
	string rvar = var_name(node->right());
	int N = node->numdim().toInt();
	add_line( var + " = (" + equal_coord_cond(N,coord) + ") ? " + rvar + " : " + lvar + ";" );
}

void Skeleton::visit(Access *node) {
	string var = var_name(node);
	Coord coord = node->cell_coord;
	string pvar = var_name(node->prev());
	int N = node->prev()->numdim().toInt();

	add_line( "if (" + equal_coord_cond(N,coord) + ") " + var + " = " + pvar + ";" );
}

void Skeleton::visit(Read *node) {
	assert(0);
}

void Skeleton::visit(Write *node) {
	assert(0);
}

void Skeleton::visit(Scalar *node) {
	assert(0);
}

void Skeleton::visit(Temporal *node) {
	// nothing to do
}

void Skeleton::visit(Checkpoint *node) {
	assert(0);
}

void Skeleton::visit(Barrier *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}

void Skeleton::visit(Summary *node) {
	add_line( var_name(node) + " = " + var_name(node->prev())+ + ";" );
}

void Skeleton::visit(DataSummary *node) {
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
	shared.push_back( std::make_pair(node,prod(ver->groupsize())) );
}

void Skeleton::visit(BlockSummary *node) {
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
	shared.push_back( std::make_pair(node,prod(ver->groupsize())) );
}

void Skeleton::visit(GroupSummary *node) {
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
	shared.push_back( std::make_pair(node,prod(ver->groupsize())) );
}

} } // namespace map::detail
