/**
 * @file	Block1.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 */

#ifndef MAP_RUNTIME_BLOCK_1_HPP_
#define MAP_RUNTIME_BLOCK_1_HPP_

#include "Block.hpp"


namespace map { namespace detail {

typedef int Berr; // Should be enum
class Node; // forward declaration
class IFile; // forward declaration

struct Block1;
typedef std::vector<Block1*> Block1List;


/*
 * @class Block1
 */
struct Block1 : public Block
{
  // Constructors
	Block1();
	Block1(Key key, int dep, cl_mem scalar_page);
	~Block1() override;

  // Getters
	int size() const override;
	HoldType holdtype() const override;

  // Methods
	Berr preload() override;
	Berr load() override;
	Berr store() override;
	Berr init() override;
	Berr reduce() override;

	Berr send();
	Berr recv();
	Berr read();
	Berr write();

	std::shared_ptr<IFile> getFile() const override;
	void setFile(std::shared_ptr<IFile> file) override;

	void* getHostMem() const override;
	void setHostMem(void *host_mem) override;
	void unsetHostMem() override;

	void* getDevMem() override;

	//CellStats getStats() const override;
	void setStats(CellStats sta) override;

	VariantType getValue() const override;
	void setValue(VariantType val) override;

	bool isFixed() const override;
	void fixValue(VariantType val) override;

  // Variables
	void *host_mem;
	std::shared_ptr<IFile> file; // @
	cl_mem scalar_page; //!< Page reserved for scalar reductions

	VariantType value;
	bool fixed; // Value of the block is fixed to a scalar (stored in 'value')
};

} } // namespace map::detail

#endif
