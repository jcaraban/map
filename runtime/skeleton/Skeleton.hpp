/**
 * @file	Skeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the graph that composes OpenCL kernel codes from skeletons
 */

#ifndef MAP_RUNTIME_SKELETON_HPP_
#define MAP_RUNTIME_SKELETON_HPP_

#include "../visitor/Visitor.hpp"
#include <unordered_map>
#include <vector>
#include <set>
#include <map>


namespace map { namespace detail {

struct Version; // forward declaration

/*
 *
 */
enum SkelPos {NONE_SKEL_POS=0x00, ALL_SKEL_POS=0x01, INPUT_OUTPUT=0x02, LOCAL_CORE=0x04,
				FOCAL_CORE=0x08, PRE_FOCAL=0x10, ZONAL_CORE=0x20, PRE_ZONAL=0x40,
				RADIAL_CORE=0x80, PRE_RADIAL=0x100, SPREAD_CORE=0x200, PRE_SPREAD=0x400,
				N_SKEL_POS=0x800};

/*
 * Tag defining the section of the skeleton where an operation occurs
 */
struct SkelTag {
	SkelPos pos; //!< PRE, CORE, POS
	int pds; //!< product of reach data size (e.g. 3x3 Mask = 9)

	void add(SkelPos pos);
	bool is(SkelPos pos) const;
	bool operator==(SkelTag tag) const;
	bool operator<(SkelTag tag) const;

	struct Hash {
		std::size_t operator()(const SkelTag& k) const;
	};
};

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

	std::string indent();
	void add_line(std::string line);
	void add_section(std::string section);
	void add_include(std::string file);

  // visit
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
	DECLARE_VISIT(BlockSummary)

  // vars
	Version *const ver; //!< Aggregation

	std::unordered_map<Node*,SkelTag> tag_hash; //!< Stores the 'tag' assigned to each node
	std::vector<SkelTag> tag_vec; //!< Stores unique-sorted 'tags' found in the nodes

	std::unordered_map<SkelTag,std::string,SkelTag::Hash> code_hash;
	SkelTag curr_tag;
	std::string full_code;
	int indent_count;

	std::array<std::vector<int>,S64+1> scalar; //!< Stores the necessary scalar declarations of each type
	std::vector<std::string> include;
	std::vector<std::string> define;

	std::vector<Node*> shared; //!< Stores those nodes whose memory is to be stored on shared-mem
	std::vector<Diversity*> diver; //!< Stores diversity nodes
	std::vector<Rand*> rand; //!< Stores rand nodes
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
