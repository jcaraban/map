/**
 * @file    ScalarTask.hpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_TASK_SCALAR_HPP_
#define MAP_RUNTIME_TASK_SCALAR_HPP_

#include "Task.hpp"
#include "../visitor/Visitor.hpp"


namespace map { namespace detail {

#define DECLARE_VISIT(class) virtual void visit(class *node);

/*
 *
 */
struct ScalarTask : public Task, public Visitor
{
	ScalarTask(Group *group);

	void createVersions();

	void blocksToLoad(Coord coord, InKeyList &in_keys) const;
	void blocksToStore(Coord coord, OutKeyList &out_keys) const;

	void initialJobs(std::vector<Job> &job_vec);
	void selfJobs(Job done_job, std::vector<Job> &job_vec);
	void nextJobs(Key done_block, std::vector<Job> &job_vec);

	int selfInterDepends(Node *node, Coord coord) const;
	int nextInterDepends(Node *node, Coord coord) const;
	int selfIntraDepends(Node *node, Coord coord) const;
	int nextIntraDepends(Node *node, Coord coord) const;

	void compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk);
	
	Pattern pattern() const { return LOCAL; }

  // visit
	bool is_input(Node *node); // @
	bool is_output(Node *node); // @

	DECLARE_VISIT(Constant)
	DECLARE_VISIT(Cast)
	DECLARE_VISIT(Unary)
	DECLARE_VISIT(Binary)
	DECLARE_VISIT(Conditional)
	DECLARE_VISIT(Diversity)
	DECLARE_VISIT(Scalar)
	DECLARE_VISIT(ZonalReduc)

  // vars
	VariantType variant; //!< Used to store the last returned value while visiting nodes
	BlockList in_blk, out_blk;  //!< Used to store the blocks while visiting nodes
};

#undef DECLARE_VISIT

} } // namespace map::detail

#endif
