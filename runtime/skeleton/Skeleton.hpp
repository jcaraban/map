/**
 * @file	Skeleton.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Visitor of the dag that composes the kernel codes from skeletons
 *
 * TODO: visit functions should 'tag' the nodes as PRE/CORE/POS and then fill() iterates them to generate the code
 */

#ifndef MAP_RUNTIME_SKELETON_HPP_
#define MAP_RUNTIME_SKELETON_HPP_

#include "../visitor/Visitor.hpp"
#include <unordered_map>


namespace map { namespace detail {

struct Version; // forward declaration

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 * Composes a program by walking the DAG from a leaf node with a BOT-UP head-recursive approach
 */
struct Skeleton : public Visitor
{
  // constructor and main function
	static Skeleton* Factory(Version *ver);

	Skeleton(Version *ver);
	virtual ~Skeleton() { };
	virtual void generate() = 0;

  // methods
	void tag(Node *node);
	void fill();
	void compact();

	std::string indent();
	void add_line(std::string line);
	void add_section(std::string section);
	void add_include(std::string file);

  // visit
	DECLARE_VISIT(Constant)
	DECLARE_VISIT(Index)
	DECLARE_VISIT(Rand)
	DECLARE_VISIT(Cast)
	DECLARE_VISIT(Unary)
	DECLARE_VISIT(Binary)
	DECLARE_VISIT(Conditional)
	DECLARE_VISIT(Diversity)
	DECLARE_VISIT(LhsAccess)
	DECLARE_VISIT(Access)
	DECLARE_VISIT(Read)
	DECLARE_VISIT(Write)
	DECLARE_VISIT(Scalar)
	DECLARE_VISIT(Temporal)
	DECLARE_VISIT(Stats)
	DECLARE_VISIT(Barrier)
	
  // vars
	Version *const ver; //!< Aggregation
  	
  	int indent_count;

	enum NodePos {ALL_POS, PRECORE, CORE, POSCORE, N_NODE_POS};
	NodePos node_pos; //!< Marks the current position where the code is to be inserted
	
	std::array<std::string,N_NODE_POS> code;

  	std::array<std::vector<int>,S64+1> scalar; //!< Stores the necessary scalar declarations of each type
	std::vector<Node*> shared; //!< Stores those nodes whose memory is to be stored on shared-mem
	std::vector<Diversity*> diver; //!< Stores diversity nodes
	std::vector<Rand*> rand; //!< Stores rand nodes

	std::vector<std::string> includes;

	std::unordered_map<Node*,NodePos> tag_hash;
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
