/**
 * @file	Block.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: study if the structs should go inside Block (e.g. Block::Key)
 * TODO: study if Block should be divided in Block0, Block1, BlockN depending on the HoldType
 */

#ifndef MAP_RUNTIME_BLOCK_HPP_
#define MAP_RUNTIME_BLOCK_HPP_

#include "Entry.hpp"
#include "../util/util.hpp"
#include "../cle/OclEnv.hpp"


namespace map { namespace detail {

typedef int Berr; // Should be enum
class Node; // forward declaration
class IFile; // forward declaration

enum HoldType { NONE_HOLD, HOLD_0, HOLD_1, HOLD_N, N_HOLD };
enum DependType { DEPEND_UNKNOWN = -1, DEPEND_ZERO = 0 };

/*
 *
 */
struct Key {
	Node *node; //!< Node owner of the block of data
	Array4<int> coord; //!< Coordinate of the block of data
	
	Key();
	Key(Node *node, Coord coord);
	bool operator==(const Key& k) const;
};

struct key_hash {
	std::size_t operator()(const Key& k) const;
};

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
  // Constructors, Destructors, Operators
	Block();
	Block(Key key);
	Block(Key key, cl_mem scalar_page);
	Block(Key key, int total_size, int depend);
	~Block();

  // Methods
	Berr send();
	Berr recv();
	Berr load(IFile *file);
	Berr store(IFile *file);

	void notify();
	bool discardable() const;

	int size() const;
	StreamDir streamdir() const;
	DataType datatype() const;
	NumDim numdim() const;
	MemOrder memorder() const;
	HoldType holdtype() const;

  // Variables
	Key key;
	Entry *entry;
	cl_mem scalar_page; //!< Points to the page reserved for scalars
	VariantType value;
	bool fixed;
	BlockStats stats;
	int total_size; //!< The total size of a single block cannot overflow an int
	int dependencies;
	HoldType hold_type;
};

typedef std::vector<Block*> BlockList;
typedef std::vector<std::tuple<Key,HoldType>> InKeyList; // Key-Holding List
typedef std::vector<std::tuple<Key,HoldType,int>> OutKeyList; // Key-Holding-Dependencies-Write list

} } // namespace map::detail

#endif
