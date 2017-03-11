/**
 * @file	Node.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: signature() is not really working smooth. A binary+ task could have 4 signatures
 *       depending on the order of nodes. However the kernel does the same in all 4 cases
 * TODO: the constructor of Nodes should be private, so that only Factory calls it
 */

#ifndef MAP_RUNTIME_DAG_NODE_HPP_
#define MAP_RUNTIME_DAG_NODE_HPP_

#include "../Pattern.hpp"
#include "../../file/MetaData.hpp"
#include "../../file/DataStats.hpp"
#include <vector>
#include <string>
#include <memory>


namespace map { namespace detail {

struct Visitor; // Forward declaration
struct Node; //!< Forward declaration
struct Group; //!< Forward declaration

typedef std::vector<std::unique_ptr<Node>> OwnerNodeList;
typedef std::vector<Node*> NodeList;
typedef std::vector<Group*> GroupList;

/*
 * Abstract Class Node
 */
struct Node {
  // Constructors & methods
	Node();
	Node(const MetaData &meta);
	//Node(Node *other); // for ::Clone ?
	virtual ~Node();

	virtual void accept(Visitor *visitor) = 0;
	virtual void acceptPrev(Visitor *visitor);
	virtual void acceptNext(Visitor *visitor);

	virtual std::string getName() const = 0;
	virtual std::string signature() const = 0;

	void increaseRef();
	void decreaseRef();

	virtual const NodeList& prevList() const;
	virtual const NodeList& nextList() const;
	
	void addNext(Node *node);
	void updatePrev(Node *old_node, Node *new_node);
	void updateNext(Node *old_node, Node *new_node);
	void removeNext(Node *node);

	virtual Pattern pattern() const; // FREE by default
	virtual bool isInput() const; // False by default
	virtual bool isOutput() const; // False by default
	virtual BlockSize halo() const; // {0..}xND by default

	const MetaData& metadata() const;
	StreamDir streamdir() const;
	DataType datatype() const;
	NumDim numdim() const;
	MemOrder memorder() const;
	const DataSize& datasize() const;
	const BlockSize& blocksize() const;
	const NumBlock& numblock() const;
	const DataStats& datastats() const;

  // Variables
	static int id_count; // Static counter to give unique ids
	int id; //!< Unique id of the node
	int ref; //!< References count

	NodeList prev_list; //!< Prev nodes on which this one depends
	NodeList next_list; //!< Next nodes depending on this one
	MetaData meta; //!< MetaData
	DataStats stats; //!< DataStats
	VariantType value; //!< Value of D0 nodes
};

} } // namespace map::detail

#endif

#include "util.hpp" // utilities for Node
