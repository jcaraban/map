/**
 * @file	Block.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: study if Block should be divided in Block0, Block1, BlockN depending on the HoldType
 */

#ifndef MAP_RUNTIME_BLOCK_HPP_
#define MAP_RUNTIME_BLOCK_HPP_

#include "Key.hpp"
#include "Entry.hpp"
#include "../util/util.hpp"
#include "../cle/OclEnv.hpp"
#include <mutex>
#include <condition_variable>


namespace map { namespace detail {

typedef int Berr; // Should be enum
class Node; // forward declaration
class IFile; // forward declaration

enum HoldType { NONE_HOLD, HOLD_0, HOLD_1, HOLD_N, N_HOLD };
enum DependType { DEPEND_UNKNOWN = -1, DEPEND_ZERO = 0 };

/*
 *
 */
struct BlockStats {
	//typedef Ctype<F64> type;
	typedef VariantUnion type;
	bool active;
	type max;
	type mean;
	type min;
	type std;

	BlockStats();
};

/*
 * @class Block
 */
struct Block 
{
  // Constructors
	Block();
	Block(Key key);
	Block(Key key, cl_mem scalar_page);
	Block(Key key, int total_size, int depend);
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
	Berr load(IFile *file);
	Berr store(IFile *file);

	void fixValue(VariantType value);

	void notify();
	bool discardable() const;

	void setReady();
	bool isReady();
	
	void setDirty();
	void unsetDirty();
	bool isDirty();

	void setUsed();
	void unsetUsed();
	bool isUsed();

	void setLoading();
	void unsetLoading();
	bool isLoading();

	void setWriting();
	void unsetWriting();
	bool isWriting();

	void waitForLoader();
	void notifyLoaders();

	void waitForWriter();
	void notifyWriters();

  // Variables
	Key key;
	std::mutex mtx;
	std::condition_variable cv_load, cv_write; //cv_evict

	Entry *entry;
	cl_mem scalar_page; //!< Points to the page reserved for scalars

	VariantType value;
	bool fixed; // Value of the block is fixed to a scalar (stored in 'value')
	bool ready; // The data is ready to be used (i.e. loaded in entry)
	BlockStats stats;

	int total_size; //!< The total size of a single block cannot overflow an int
	int dependencies;
	HoldType hold_type;

	char used;
	bool dirty, loading, writing;
};

typedef std::vector<Block*> BlockList;
typedef std::vector<std::tuple<Key,HoldType,int>> InKeyList; // Key-Holding-Dependencies List
typedef std::vector<std::tuple<Key,HoldType,int>> OutKeyList; // Key-Holding-Dependencies list

} } // namespace map::detail

#endif
