/**
 * @file    LoopTask.cpp 
 * @author  Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#include "LoopTask.hpp"
#include "../Runtime.hpp"


namespace map { namespace detail {

LoopTask::LoopTask(Program &prog, Clock &clock, Config &conf, Group *group)
	: Task(prog,clock,conf,group)
{
	this->cond_node = nullptr;
	pat = NONE_PATTERN;

	for (auto node : group->nodeList()) {
		// @ better way of extracting the nodes than dynamic_cast?
		if (node->pattern().is(LOOP)) {
			assert(this->cond_node == nullptr);
			this->cond_node = dynamic_cast<LoopCond*>(node);
			assert(this->cond_node != nullptr);
		}
		pat += node->pattern();
	}
}

void LoopTask::createVersions() {
	cle::OclEnv& env = Runtime::getOclEnv();
	
	assert(0);
}

void LoopTask::blocksToLoad(Coord coord, KeyList &in_keys) const {
	in_keys.clear();

	// @@ Input nodes depend on the notifiying task: head or tail

	for (int i=0; i<inputList().size(); i++)
	{
		Node *node = inputList()[i];
		const int N = is_input_of[i].is(LOOP) ? 1 : 0;
		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		
		for (int y=-N; y<=N; y++) {
			for (int x=-N; x<=N; x++) {
				auto nbc = coord + Coord{x,y};
				HoldType hold_nbc = (any(nbc < 0) || any(nbc >= numblock())) ? HOLD_0 : hold;
				int depend = node->isInput() ? nextInterDepends(node,nbc) : -1; // @
				in_keys.push_back( std::make_tuple(Key(node,nbc),hold_nbc,depend) );
			}
		}
	}
}

void LoopTask::blocksToStore(Coord coord, KeyList &out_keys) const {
	out_keys.clear();

	// All non-LOOP outputs first
	for (int i=0; i<outputList().size(); i++)
	{
		Node *node = outputList()[i];

		if (node->pattern().is(LOOP))
			continue; // No spread node now

		HoldType hold = (node->numdim() == D0) ? HOLD_1 : HOLD_N;
		int depend = -1; // non-discardable because unknown dependencies and stability

		out_keys.push_back( std::make_tuple(Key(node,coord),hold,depend) );
	}

}

void LoopTask::initialJobs(std::vector<Job> &job_vec) {
	Task::initialJobs(job_vec);
}

void LoopTask::askJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);	

	
	// unstable
	{
		// Asks itself for self-jobs, a.k.a. intra-dependencies (e.g. Spread, Radial)
		this->selfJobs(done_job,job_vec);
	}
	// stable
	{
		// Asks next-tasks for their next-jobs, a.k.a inter-dependencies (all Op)
		for (auto next_task : this->nextList()) {
			auto common_nodes = inner_join(this->outputList(),next_task->inputList());
			for (auto node : common_nodes) {
				if (node->numdim() == D0)
					continue; // D0 jobs only notify once
				Key key = Key(node,done_job.coord);
				next_task->nextJobs(key,job_vec);
			}
		}
	}
}

void LoopTask::selfJobs(Job done_job, std::vector<Job> &job_vec) {
	assert(done_job.task == this);

	for (int y=-1; y<=1; y++)
		for (int x=-1; x<=1; x++)
			notify(done_job.coord+Coord{x,y},job_vec);
}

void LoopTask::nextJobs(Key done_block, std::vector<Job> &job_vec) {
	if (done_block.node->numdim() == D0) // Case when prev=D0, self=D2
	{
		notifyAll(job_vec);
	}
	else // Case when prev=D2, self=D2
	{
		int pos = value_position(done_block.node,inputList());
		const int N = is_input_of[pos].is(LOOP) ? 1 : 0;

		for (int y=-N; y<=N; y++) {
			for (int x=-N; x<=N; x++) {
				auto nbc = done_block.coord + Coord{x,y};
				if (all(nbc >= 0) && all(nbc < numblock())) {
					notify(nbc,job_vec);
				}
			}
		}
	}
}

int LoopTask::prevInterDepends(Node *node, Coord coord) const {
	//	return 0; // There are no inter-dependencies after the first initial job

	int pos = value_position(node,inputList());
	if (!is_input_of[pos].is(LOOP))
		return node->pattern() == FREE ? 0 : 1;
	
	// Spread inputs depend on their neighborhood
	int depend = 0;
	for (int y=-1; y<=1; y++) {
		for (int x=-1; x<=1; x++) {
			Coord nbc = coord + Coord{x,y};
			if (all(nbc >= 0) && all(nbc < numblock()))
				depend += node->isInput() ? 0 : 1;
		}
	}
	return depend;
}

int LoopTask::nextInterDepends(Node *node, Coord coord) const {
	return prevInterDepends(node,coord); // @ reusing prevInterDepends, but would need own code
}

int LoopTask::prevIntraDepends(Node *node, Coord coord) const {
	// After the first initial job, even 1 self-job should be able to activate this job
	// de cuantos self-blocks dependo yo?
}

int LoopTask::nextIntraDepends(Node *node, Coord coord) const {
	return -1; // Unknown
	// cuantos self-blocks dependen de mi?
}

void LoopTask::compute(Coord coord, const BlockList &in_blk, const BlockList &out_blk) {
	const Version *ver = getVersion(DEV_ALL,{},""); // Any device, no detail
	cle::Task tsk = ver->tsk;
	cle::Queue que = tsk.C().D(Tid.dev()).Q(Tid.rnk());

	// If the out-block is stable, make sure to pass 'write=true' to release-Output-Block
}

Pattern LoopTask::pattern() const {
	return pat;
}

} } // namespace map::detail
