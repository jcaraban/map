/**
 * @file	Skeleton.cpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "Skeleton.hpp"
#include "skeleton/RadialSkeleton.hpp"
#include "skeleton/LoopSkeleton.hpp"
#include "Version.hpp"
#include "Task.hpp"
#include <iostream>
#include <functional>
#include <queue>


namespace map { namespace detail {

namespace { // anonymous namespace
	using std::string;
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

	return generateCode();
}

Section* Skeleton::newSection(Node *node, Pattern pat) {
	DataSize ext = node->outputReach().datasize();
	Section *new_sect = Section::Factory(ext,pat,this);
	owner_list.push_back( std::unique_ptr<Section>(new_sect) );
	return new_sect;
}

Section* Skeleton::newSection(Section *sect) {
	Section *new_sect = Section::Factory(sect->ext,sect->pat,sect->skel);
	*new_sect = *sect;
	owner_list.push_back( std::unique_ptr<Section>(new_sect) );
	return new_sect;
}

void Skeleton::deleteSection(Section *section) {
	auto pred = [&](const std::unique_ptr<Section> &g) { return g.get() == section; };
	owner_list.erase(std::remove_if(owner_list.begin(),owner_list.end(),pred),owner_list.end());
}

/***********
   Methods
 ***********/

void Skeleton::tag() {
	// Tags in + body + out nodes according to their pattern
	struct NodeTag {
		Node *node;
		Pattern pat;
	};
	std::vector<NodeTag> tag_list;

	for (auto node : ver->task->inputList())
		tag_list.push_back( NodeTag{node,INPUT} );
	for (auto node : ver->task->nodeList())
		tag_list.push_back( NodeTag{node,node->pattern()} );
	for (auto node : ver->task->outputList())
		tag_list.push_back( NodeTag{node,OUTPUT} );

	auto body_out = full_unique_join(ver->task->nodeList(),ver->task->outputList());

	// Walks nodes backward and accumulate their 'section = { Pattern , prod_data_size }'
	for (auto it=tag_list.rbegin(); it!=tag_list.rend(); it++) {
		Node *node = it->node;

		Section *section = newSection(node,it->pat);

		// Only 'next' nodes inside the 'task's body
		auto next_node_list = inner_join(node->nextList(),body_out);
		// Special cases when the node is body & output
		if (is_included(node,ver->task->outputList())) {
			if (section->pat.is(OUTPUT))
				next_node_list.clear();
			else // pat isNot OUTPUT
				next_node_list.push_back(node);
		}

		// For every next node, iterate their sections
		for (auto next_node : next_node_list) {
			// For every next section, generally non-OUTPUT
			for (auto next_sect : section_list_of[next_node]) {
				if (next_node->pattern().isNot(OUTPUT) && 
					next_sect->is(OUTPUT) && node != next_node)
					continue;
				// Creates a partial section wrt next node
				Section *partial = newSection(section);
				auto next_ext = next_node->inputReach().datasize();

				// @@ Otherwise Output-Focals spread their reach twice
				if (next_sect->is(OUTPUT))
					next_ext = next_node->numdim().unitVec();

				// @@
				if (partial->ext.size() != 0)
					partial->ext = max(next_ext,next_sect->ext);

				// Early unification of sections
				if (section_set.find(partial) == section_set.end()) {
					section_set.insert(partial);
					section_list.push_back(partial);
				}
				//
				Section *unified = *section_set.find(partial);
				//
				if (not is_included(node,unified->node_list)) {
					section_list_of[node].push_back(unified);
					unified->node_list.push_back(node);
				}
				//
				if (unified == next_sect)
					continue;
				//
				if (not is_included(next_sect,unified->next_list))
					unified->next_list.push_back(next_sect);
				//
				if (not is_included(unified,next_sect->prev_list))
					next_sect->prev_list.push_back(unified);
			}
		}
		// Outputs have no next nodes, just create the Section
		if (next_node_list.empty()) {
			if (section_set.find(section) == section_set.end()) {
				section_set.insert(section);
				section_list.push_back(section);
			}

			Section *unified = *section_set.find(section);

			if (not is_included(node,unified->node_list)) {
				section_list_of[node].push_back(unified);
				unified->node_list.push_back(node);
			}
		}
	}

	assert(not section_list.empty());

	// Reversing list to sort nodes
	for (auto sect : section_list)
		std::reverse(sect->node_list.begin(),sect->node_list.end());

	// Topological sort of sections
	section_list = toposort(section_list);

	assert(not section_list.empty());
}

SectionList Skeleton::toposort(SectionList list) {
	assert(not list.empty());

	struct less {
		bool operator()(Section *a, Section *b) { return *a < *b; }
	};

	std::priority_queue<Section*,std::vector<Section*>,less> prique;
	std::unordered_map<Section*,int,Section::Hash,Section::Equal> prev_count;
	SectionList ordered_list;

	// Walks all 'sects' first to registers their 'prevs'
	for (auto sect : list) {
		int count = sect->prevList().size();
		prev_count.insert({sect,count});
		
		if (count == 0) { // ready 'sects' with 'prev = 0' go into the prique
			prique.push(sect);
			prev_count.erase(sect);
		}
	}

	// (1) 'queue' for the bfs topo-sort, (2) 'priority' for the sect-sort
	while (not prique.empty()) {
		Section *sect = prique.top();
		prique.pop();

		ordered_list.push_back(sect);

		for (auto next : sect->nextList()) {
			auto it = prev_count.find(next);
			assert(it != prev_count.end());
			auto &count = it->second;

			count--;
			if (count == 0) {
				prique.push(next);
				prev_count.erase(next);
			}
		}	
	}

	assert(prev_count.empty());
	return ordered_list;
}

void Skeleton::fill() {
	// Puts in + body + out nodes into 'all_list', in order
	auto in_body = full_join(ver->task->inputList(),ver->task->nodeList());
	auto all_list = full_unique_join(in_body,ver->task->outputList());

	full_code.clear();
	indent_count = 1;

	// Reserves some decent length for the strings
	const int avg_skel_len = 4096;	
	for (auto sect : section_list)
		sect->code.reserve(avg_skel_len);

	// Walk nodes to fill section's codes
	for (auto sect : section_list)
		sect->fill();

	// Walks in + body nodes, registering the necessary scalar variables
	for (auto node : in_body)
		scalar[ node->datatype().get() ].push_back(node->id);

	// @@ Copy section variables to skeleton, bad desing?
	for (auto sect : section_list) {
		include.insert(include.end(),sect->include.begin(),sect->include.end());
		//
		ext_shared.insert(ext_shared.end(),sect->ext_shared.begin(),sect->ext_shared.end());
		shared.insert(shared.end(),sect->shared.begin(),sect->shared.end());
		mask.insert(mask.end(),sect->mask.begin(),sect->mask.end());
		//
		diver.insert(diver.end(),sect->diver.begin(),sect->diver.end());
		radia.insert(radia.end(),sect->radia.begin(),sect->radia.end());
		reduc_list.insert(reduc_list.end(),sect->reduc_list.begin(),sect->reduc_list.end());
		merge_list.insert(merge_list.end(),sect->merge_list.begin(),sect->merge_list.end());
		switch_list.insert(switch_list.end(),sect->switch_list.begin(),sect->switch_list.end());
	}

	indent_count = -1;
}

void Skeleton::compact() {
	// Cleaning unaccessible sections
	auto pred_rem = [&](std::unique_ptr<Section> &sect){ return not is_included(sect.get(),section_list); };
	owner_list.erase(std::remove_if(owner_list.begin(),owner_list.end(),pred_rem),owner_list.end());

	// 
	for (auto sect : section_list) {
		if (sect->ext.size() > extension_list.size())
			extension_list.resize( sect->ext.size() );
	}

	//sort_unique(diver,node_id_less(),node_id_equal());

	sort_unique(ext_shared);
	// Transfers nodes to 'shared' structure
	for (auto pair : ext_shared) {
		int shared_size = prod(ver->groupsize() + pair.ext / 2 * 2);
		shared.push_back( std::make_pair(pair.node,shared_size) );
	}

	// Compacts 'shared' structure
	/*
	typedef std::pair<Node*,int> pair;
	auto greater_pred = [](pair lhs, pair rhs) { return lhs.first->id > rhs.first->id; };
	auto equal_pred = [](pair lhs, pair rhs) { return lhs.first->id == rhs.first->id; };
	sort_unique(shared,less_pred,equal_pred);
	*/
}

std::string Skeleton::generateCode() {
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
	if (pattern.is(FOCAL)) {
		add_section( defines_focal() );
		add_line( "" );
	}

	bool local_types[N_DATATYPE] = {};
	bool focal_types[N_DATATYPE] = {};
	bool diver_types[N_DATATYPE] = {};
	bool reduc_def[N_DATATYPE][N_REDUCTION] = {};
	bool out_types[N_DATATYPE] = {};

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
	for (auto node : ver->task->outputList()) {
		DataType dt = node->datatype();
		out_types[dt.get()] = true;
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
		if (out_types[dt])
			add_section( defines_output_type(dt) );
	if (any_true(out_types,N_DATATYPE))
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
		if (node->numdim() == D0 && not node->isReduction())
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

	//// Code sections ////
	Section *last = nullptr;

	for (auto sect : section_list)
	{
		if (last && last->extendedReach() && *last > *sect) {
			add_section( extended_section_bot(last->ext) );
		}
		if (sect->extendedReach() && (not last || *sect > *last)) {
			add_section( extended_section_top(sect->ext) );
		}
		last = sect;

		add_section( indented( sect->generateCode() ) );
	}

	indent_count--;
	add_line( "}" ); // Closes kernel body

	//// Printing ////
	std::cout << "***\n" << full_code << "***" << std::endl;

	return full_code;
}

// 

string Skeleton::extended_section_top(DataSize ext) {
	int N = ver->task->numdim().toInt();
	string str;

	str += format( "// Extended section" );
	str += format( "{" );
	indent_count++;

	// Extension of the halos
	for (int n=0; n<N; n++) {
		str += format( string("int H")+n + " = " + ext[n]/2 + ";" );
	}
	str += format( "" );

	// Needs to loop because there are fewer threads than extended cells
	str += format( "for ("+pre_load_loop(N)+")" );
	str += format( "{" );
	indent_count++;

	// Displaced indexing variables
	str += format( "int proj = "+local_proj(N)+" + i*("+group_size_prod(N)+");" );
	str += format( "if (proj >= "+group_size_prod_H(N)+") continue;" );
	for (int n=0; n<N; n++) {
		str += format( string("int gc")+n+" = proj % ("+group_size_prod_H(n+1)+") / "+group_size_prod_H(n)+";" );
	}
	for (int n=0; n<N; n++) {
		str += format( string("int bc")+n+" = GC"+n+"*GS"+n+" + gc"+n+" - H"+n+";" );
	}
	str += format( "" );

	return str;
}

string Skeleton::extended_section_bot(DataSize ext) {
	int N = ver->task->numdim().toInt();
	string str;

	// Filling focal shared memory
	for (auto pair : ext_shared) {
		if (any(pair.ext != ext))
			continue;
		string svar = var_name(pair.node,SHARED) + "[" + local_proj_focal(N) + "]";
		string var = var_name(pair.node);
		str += format( svar + " = " + var + ";" );
	}
	
	// Closes load-loop
	indent_count--;
	str += format( "}" );
	// Synchronizes
	str += format( "barrier(CLK_LOCAL_MEM_FENCE);" );
	// Closes extended section
	indent_count--;
	str += format( "}" );
	str += format( "" );

	return str;
}

//

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

string Skeleton::format(string str) {
	return indent() + str + "\n";
}

void Skeleton::add_line(string line) {
	full_code += indent() + line + "\n";
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

} } // namespace map::detail
