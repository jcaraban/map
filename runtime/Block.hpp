/**
 * @file	Block.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: speciallizing Block by 'datatype' and 'holdtype' could reduce the runtime overhead
 */

#ifndef MAP_RUNTIME_BLOCK_HPP_
#define MAP_RUNTIME_BLOCK_HPP_

#include "Key.hpp"
#include "Entry.hpp"
#include "../file/DataStats.hpp"
#include "../util/util.hpp"
#include "../cle/OclEnv.hpp"
#include <mutex>
#include <condition_variable>


namespace map { namespace detail {

typedef int Berr; // Should be enum
class Node; // forward declaration
class IFile; // forward declaration

struct Block;
typedef std::vector<Block*> BlockList;


/*
 * @class Block
 */
struct Block
{
  // Constructors
	Block();
	Block(Key key, int dep);
	Block(Key key, int dep, cl_mem scalar_page, cl_mem group_page);
	//Block(Key key, int dep, cl_mem group_page, int size);
	Block(Key key, int dep, int total_size);
	~Block();

  // Getters
	int size() const;
	StreamDir streamdir() const;
	DataType datatype() const;
	NumDim numdim() const;
	MemOrder memorder() const;
	HoldType holdtype() const;

  // Methods
	Berr send();
	Berr recv();
	Berr load();
	Berr store();

	void fixValue(VariantType val);
	void setStats(CellStats sta);
	void forwardEntry(Block *out);

	void notify();
	bool discardable() const;

	void setReady();
	void unsetReady();
	bool isReady();
	
	void setDirty();
	void unsetDirty();
	bool isDirty();

	void setUsed();
	void unsetUsed();
	bool isUsed();

  // Variables
	Key key;

	Entry *entry;
	void *host_mem;
	std::shared_ptr<IFile> file; // @
	cl_mem scalar_page; //!< Page reserved for scalar reductions
	cl_mem group_page; //!< Page reserved for group statistics

	VariantType value;
	bool fixed; // Value of the block is fixed to a scalar (stored in 'value')
	bool forwarded; // The memory entry will be forwarded from its input node
	bool ready; // The data is ready to be used (i.e. loaded in entry)
	BlockStats stats;

	int total_size; //!< The total size of a single block cannot overflow an int
	int dependencies;
	HoldType hold_type;
	short used;
	bool dirty;
	char order;

	std::mutex mtx;
};

} } // namespace map::detail

#endif
