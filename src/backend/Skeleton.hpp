/**
 * @file	Skeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * For/back/ward visitor of the graph that composes OpenCL kernel codes from skeletons
 *
 * TODO: SERIOUS CLEAN UP
 */

#ifndef MAP_RUNTIME_SKELETON_HPP_
#define MAP_RUNTIME_SKELETON_HPP_

#include "Section.hpp"
#include "util.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace map { namespace detail {

struct Version; // forward declaration

/*
 * Walks the graph to compose an OpenCL GPU kernel
 */
struct Skeleton
{
  // constructor and main function
	static Skeleton* Factory(Version *ver);

	Skeleton(Version *ver);
	virtual ~Skeleton() { };
	virtual std::string generate();

	Section* newSection(Node *node, Pattern pat);
	Section* newSection(Section *sect);
	void deleteSection(Section *sect);

  // methods
	void tag();
	void fill();
	void compact();
	SectionList toposort(SectionList list);

	std::string generateCode();

	std::string extended_section_top(DataSize ext);
	std::string extended_section_bot(DataSize ext);

	std::string indent();
	std::string indented(std::string code);
	std::string format(std::string str);
	void add_line(std::string line);
	void add_section(std::string section);
	void add_include(std::string file);

  // vars
	Version *const ver; //!< Aggregation

	OwnerSectionList owner_list;
	SectionList section_list; //!< Stores unique-sorted 'sections' derived from the nodes
	std::unordered_map<Node*,SectionList> section_list_of; //!< Stores the 'section' assigned to each node
	std::unordered_set<Section*,Section::Hash,Section::Equal> section_set; //!< Keeps a list of unique (not repeated) 'sections'
	std::vector<SectionList> extension_list;

	std::string full_code;
	int indent_count;

	std::array<std::vector<int>,N_DATATYPE> scalar; //!< Stores the necessary scalar declarations of each type
	std::vector<std::string> include;
	//std::vector<std::string> define;

	std::vector<NodeExtension> ext_shared; //!< Nodes accessed in extended spatial e.g. Focal inputs
	std::vector<std::pair<Node*,int>> shared; //!< Stores pairs {node,size} for nodes requiring shared-mem chunks
	std::vector<std::pair<Mask,int>> mask; //!< Stores pairs {mask,id}

	std::vector<Diversity*> diver; //!< Stores diversity nodes
	std::vector<RadialScan*> radia; //!< Stores RadialScan nodes
	//std::vector<Rand*> rand; //!< Stores rand nodes
	std::vector<SkelReduc> reduc_list; //!< Stores reductions nodes
	std::vector<Merge*> merge_list;
	std::vector<Switch*> switch_list;
};

} } // namespace map::detail

#endif
