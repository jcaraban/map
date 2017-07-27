/**
 * @file	BlockList.hpp
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 */

#ifndef MAP_RUNTIME_BLOCK_LIST_HPP_
#define MAP_RUNTIME_BLOCK_LIST_HPP_

#include "Block.hpp"
#include <vector>


namespace map { namespace detail {

/*
 *
 */
class BlockList
{
  private:
	std::vector<Block*> list;

	typedef decltype(list)::value_type value_type;
	typedef decltype(list):: size_type  size_type;
	typedef decltype(list):: reference  reference;
	typedef decltype(list)::  iterator   iterator;

	typedef decltype(list)::const_reference const_reference;
	typedef decltype(list)::const_iterator const_iterator;

  public:
	iterator begin() { return list.begin(); }
	iterator end() { return list.end(); }
	const_iterator begin() const { return list.begin(); }
	const_iterator end() const { return list.end(); }

	size_type size() const { return list.size(); }
	void resize(size_type n) { list.resize(n); }
	void reserve(size_type n) { list.reserve(n); }
	void clear() { list.clear(); }

	void push_back(const value_type& value) { list.push_back(value); }
	reference back() { return list.back(); }
	//reference operator[](size_type n) { return list[n]; }
	const_reference operator[] (size_type n) const { return list[n]; }
  	//cosnt_reference back() { return list.back(); }

	bool include(const value_type& value) const { 
		return std::find(list.begin(),list.end(),value) != list.end();
	}

  // methods
	void preloadInputs();
	void evictEntries();
	void loadInputs();
	void initOutputs();
	void storeOutputs();
	void reduceOutputs();
};

} } // namespace map::detail

#endif