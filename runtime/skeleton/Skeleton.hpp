/**
 * @file	Skeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the graph that composes OpenCL kernel codes from skeletons
 *
 * TODO: rename "tag" to "section", like in <Code Section> ?
 * TODO: clean up of declared variables and data structures ?
 * TODO: SERIOUS CLEAN UP
 */

#ifndef MAP_RUNTIME_SKELETON_HPP_
#define MAP_RUNTIME_SKELETON_HPP_

#include "util.hpp"
#include "../visitor/Visitor.hpp"
#include <unordered_map>
#include <vector>
#include <set>
#include <map>
#include <queue>


namespace map { namespace detail {

struct Version; // forward declaration

/*
 *
 */
enum SkelPos { NONE_SKEL_POS=0x00, INPUT_POS=0x01, OUTPUT_POS=0x02, FREE_POS=0x04, LOCAL_POS=0x08,
				FOCAL_POS=0x10, ZONAL_POS=0x20, RADIAL_POS=0x40, SPREAD_POS=0x80, N_SKEL_POS=0x100 };

constexpr SkelPos operator+(const SkelPos& lhs, const SkelPos& rhs) {
	return static_cast<SkelPos>( static_cast<int>(lhs) | static_cast<int>(rhs) );
}

/*
 * Tag defining the section of the skeleton where an operation occurs
 */
struct SkelTag {
	Pattern pat; //!< Absolute position according to the section pattern
	//Pattern prev, next; //!< Relative position according to prev-next patterns
	DataSize ext; //!< Extension of the 'spatial reach' needed in this section

	bool extendedReach() const;
	void add(Pattern pat);
	void sub(Pattern pat);
	bool is(Pattern pat) const;
	bool isNot(Pattern pat) const;

	bool operator==(const SkelTag &tag) const;
	bool operator<(const SkelTag &tag) const;
	bool operator>(const SkelTag &tag) const;

	struct Hash {
		std::size_t operator()(const SkelTag& k) const;
	};
};

typedef std::vector<SkelTag> TagList;


#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Walks the graph to compose an OpenCL GPU kernel
 */
struct Skeleton : public Visitor
{
  // constructor and main function
	static Skeleton* Factory(Version *ver);

	Skeleton(Version *ver);
	virtual ~Skeleton() { };
	virtual std::string generate();

  // methods
	void tag();
	void fill();
	void compact();
	std::string versionCode();

	TagList sort(TagList list);

	std::string indent();
	std::string indented(std::string code);
	void add_line(std::string line);
	void add_section(std::string section);
	void add_include(std::string file);

  // code
	void dispatch_section(SkelTag tag);
	void reach_top_section(SkelTag tag);
	void reach_bot_section(SkelTag tag);
	void input_section(SkelTag tag);
	void output_section(SkelTag tag);
	void free_section(SkelTag tag);
	void local_section(SkelTag tag);
	void focal_section(SkelTag tag);
	void zonal_section(SkelTag tag);
	void stats_section(SkelTag tag);
	void loop_section(SkelTag tag);
	void reduc_section(SkelTag tag);

  // visit
	virtual void visit_input(Node *node);
	virtual void visit_output(Node *node);
	DECLARE_VISIT(Constant)
	DECLARE_VISIT(Index)
	DECLARE_VISIT(Identity)
	DECLARE_VISIT(Rand)
	DECLARE_VISIT(Cast)
	DECLARE_VISIT(Unary)
	DECLARE_VISIT(Binary)
	DECLARE_VISIT(Conditional)
	DECLARE_VISIT(Diversity)
	DECLARE_VISIT(Neighbor)
	DECLARE_VISIT(BoundedNeighbor)
	DECLARE_VISIT(SpreadNeighbor)
	DECLARE_VISIT(Convolution)
	DECLARE_VISIT(FocalFunc)
	DECLARE_VISIT(FocalPercent)
	DECLARE_VISIT(ZonalReduc)
	DECLARE_VISIT(RadialScan)
	DECLARE_VISIT(SpreadScan)
	DECLARE_VISIT(LoopCond)
	DECLARE_VISIT(LoopHead)
	DECLARE_VISIT(LoopTail)
	DECLARE_VISIT(Merge)
	DECLARE_VISIT(Switch)
	DECLARE_VISIT(Access)
	DECLARE_VISIT(LhsAccess)
	DECLARE_VISIT(Read)
	DECLARE_VISIT(Write)
	DECLARE_VISIT(Scalar)
	DECLARE_VISIT(Temporal)
	DECLARE_VISIT(Checkpoint)
	DECLARE_VISIT(Barrier)
	DECLARE_VISIT(Summary)
	DECLARE_VISIT(DataSummary)
	DECLARE_VISIT(BlockSummary)
	DECLARE_VISIT(GroupSummary)

  // vars
	Version *const ver; //!< Aggregation

	TagList tag_list; //!< Stores unique-sorted 'tags' found in the nodes
	std::unordered_map<Node*,TagList> tag_hash; //!< Stores the 'tag' assigned to each node
	std::unordered_set<SkelTag,SkelTag::Hash> tag_set; //!< Keeps a list of unique (not repeated) 'tags'

	std::unordered_map<SkelTag, std::unordered_set<SkelTag,SkelTag::Hash> ,SkelTag::Hash> prev_of; // @
	std::unordered_map<SkelTag, std::unordered_set<SkelTag,SkelTag::Hash> ,SkelTag::Hash> next_of; // @
	std::priority_queue<SkelTag,std::vector<SkelTag>,std::greater<SkelTag>> prique; // @
	std::unordered_map<SkelTag,int,SkelTag::Hash> prev_count; // @

	std::unordered_map<SkelTag,NodeList,SkelTag::Hash> node_list_of; //!< Stores the 'nodes' belonging to a 'tag'
	std::unordered_map<SkelTag,std::string,SkelTag::Hash> code_hash;
	SkelTag curr_tag;
	std::string full_code;
	int indent_count;

	std::array<std::vector<int>,N_DATATYPE> scalar; //!< Stores the necessary scalar declarations of each type
	std::vector<std::string> include;
	std::vector<std::string> define;

	std::vector<Node*> ext_shared; //!< Nodes accessed in extended spatial reach require shared-mem, e.g. inputs to Focal
	std::vector<std::pair<Node*,int>> shared; //!< Stores pairs {node,size} for nodes requiring shared-mem chunks
	std::vector<std::pair<Mask,int>> mask; //!< Stores pairs {mask,id}

	std::vector<Diversity*> diver; //!< Stores diversity nodes
	std::vector<Rand*> rand; //!< Stores rand nodes
	std::vector<Merge*> merge_list;
	std::vector<Switch*> switch_list;
	std::vector<SkelReduc> reduc_list; //!< Stores reductions nodes
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
