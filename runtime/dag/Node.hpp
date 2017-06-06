/**
 * @file	Node.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * TODO: signature() is not really working smooth. A binary+ task could have 4 signatures
 *       depending on the order of nodes. However the kernel does the same in all 4 cases
 * TODO: is 'back'/'forw'List necessary, can we use 'next'/'prev'List only with 'ssa_id' ?
 * TODO: updatePrev() sounds like a hack, get rid of it, nodes should be inmutable entities
 * TODO: move 'static id_count' somewhere outside node, to runtime? to 'session'?
 *
 * TODO: a specialized 'Back' or 'Next' node might save the need of forw / back edges everywhere
 */

#ifndef MAP_RUNTIME_DAG_NODE_HPP_
#define MAP_RUNTIME_DAG_NODE_HPP_

#include "../Key.hpp"
#include "../Pattern.hpp"
#include "../../file/MetaData.hpp"
#include "../../file/DataStats.hpp"
#include "../../util/ReductionType.hpp"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>


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
  // Constructors
	Node();
	Node(const MetaData &meta);
	virtual ~Node();
  // Clone constructor
	Node(const Node *other, const std::unordered_map<Node*,Node*> &other_to_this);
	virtual Node* clone(const std::unordered_map<Node*,Node*> &other_to_this) = 0;
  // Visitor methods
	virtual void accept(Visitor *visitor) = 0;
	virtual void acceptPrev(Visitor *visitor);
	virtual void acceptNext(Visitor *visitor);
  // Readable, Unique
	virtual std::string getName() const = 0;
	virtual std::string signature() const = 0;
  // Referenced by
	void increaseRef();
	void decreaseRef();
  // Links to prev / next nodes
	virtual const NodeList& prevList() const;
	virtual const NodeList& nextList() const;
	const NodeList& backList() const;
	const NodeList& forwList() const;
  // Prev / Next methods
	void addPrev(Node *node);
	void addNext(Node *node);
	void updatePrev(Node *old_node, Node *new_node); // @
	void updateNext(Node *old_node, Node *new_node);
	void removeNext(Node *node);
  // 
	void addBack(Node *node);
	void addForw(Node *node);
	void removeBack(Node *node);
  // Features
	virtual bool isInput() const; // False by default
	virtual bool isOutput() const; // False by default
	virtual bool canForward() const; // False by default
	virtual bool isConstant() const; // False by default
	virtual bool isReduction() const; // @
	virtual ReductionType reductype() const;
  // Spatial
	virtual Pattern pattern() const;
	virtual const Mask& inputReach(Coord coord=Coord()) const;
	//virtual const Mask& intraReach(Coord coord=Coord()) const;
	virtual const Mask& outputReach(Coord coord=Coord()) const;
	virtual HoldType holdtype(Coord coord);
  // Getters
	const MetaData& metadata() const;
	StreamDir streamdir() const;
	DataType datatype() const;
	NumDim numdim() const;
	MemOrder memorder() const;
	const DataSize& datasize() const;
	const BlockSize& blocksize() const;
	const NumBlock& numblock() const;
	const GroupSize& groupsize() const;
	const NumGroup& numgroup() const;
	const DataStats& datastats() const;
  // Value
	virtual VariantType initialValue() const;
	virtual void updateValue(VariantType value);
  // Compute
	virtual void computeScalar(std::unordered_map<Node*,VariantType> &hash);
	virtual void computeFixed(Coord coord, std::unordered_map<Key,ValFix,key_hash> &hash);

  // Variables
	int id; //!< Unique id of the node
	int ref; //!< References count

	NodeList prev_list; //!< Prev nodes on which this one depends
	NodeList next_list; //!< Next nodes depending on this one
	NodeList back_list; //!< Back nodes: next nodes with lower id (due to a loop)
	NodeList forw_list; //!< Forward " : prev nodes with higher id (see Merge nodes)
	MetaData meta; //!< MetaData
	DataStats stats; //!< DataStats
	VariantType value; //!< Value of D0 nodes

	Pattern spatial_pattern;
	Mask in_spatial_reach, out_spatial_reach;
};

} } // namespace map::detail

#endif

#include "util.hpp" // utilities for Node
