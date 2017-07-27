/**
 * @file	BlockN.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: unify 'value', 'fixed' and 'stats' in a single ´valfix'-like structure?
 */

#ifndef MAP_RUNTIME_BLOCK_N_HPP_
#define MAP_RUNTIME_BLOCK_N_HPP_

#include "Block.hpp"


namespace map { namespace detail {

/*
 * @class BlockN
 */
struct BlockN : public Block
{
  // Constructors
	BlockN();
	BlockN(Key key, int dep, int total_size);
	~BlockN() override;

  // Getters
	int size() const override;
	HoldType holdtype() const override;

  // Methods
	Berr preload() override;
	Berr evict() override;
	Berr load() override;
	Berr store() override;

	Berr send();
	Berr recv();
	Berr read();
	Berr write();

	bool needEntry() const override;
	bool giveEntry() const override;

	Entry* getEntry() const override;
	void setEntry(Entry *entry) override;
	void unsetEntry() override;

	std::shared_ptr<IFile> getFile() const override;
	void setFile(std::shared_ptr<IFile> file) override;

	void* getHostMem() const override;
	void setHostMem(void *host_mem) override;
	void unsetHostMem() override;

	void* getDevMem() override;

	CellStats getStats() const override;
	void setStats(CellStats sta) override;

	VariantType getValue() const override;
	void setValue(VariantType val) override;

	bool isFixed() const override;
	void fixValue(VariantType val) override;

	void setForward() override;
	void unsetForward() override;
	bool isForward() const override;
	void forwardEntry(Block *out) override;

  // Variables
	Entry *entry;
	void *host_mem;
	std::shared_ptr<IFile> file; // @

	VariantType value;
	bool fixed; // Value of the block is fixed to a scalar (stored in 'value')
	bool forward; // The memory entry will be forwarded from its input node
	BlockStats stats;

	int total_size; //!< The total size of a single block cannot overflow an int
};

} } // namespace map::detail

#endif
