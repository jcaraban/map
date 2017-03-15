/**
 * @file	Diversity.hpp 
 * @author	Jesús Carabaño Bravo <jcaraban@abo.fi>
 *
 * Node representing a Local Diversity operation (e.g. variance, majority, minority, mean)
 */

#ifndef MAP_RUNTIME_DAG_DIVERSITY_HPP_
#define MAP_RUNTIME_DAG_DIVERSITY_HPP_

#include "Node.hpp"


namespace map { namespace detail {

struct Diversity : public Node
{
	// Internal declarations
	struct Key {
		Key(Diversity *node);
		bool operator==(const Key& k) const;
		NodeList prev_list;
		DiversityType type;
	};
	struct Hash {
		std::size_t operator()(const Key& k) const;
	};

	// Factory
	static Node* Factory(NodeList prev_list, DiversityType type);
	Node* clone(NodeList new_prev_list);

	// Constructors
	Diversity(const MetaData &meta, NodeList prev_list, DiversityType type);
	Diversity(const Diversity *other, NodeList new_prev_list);

	// Methods
	void accept(Visitor *visitor);
	std::string getName() const;
	std::string signature() const;
	char classSignature() const;
	Node*& prev(int i);
	const Node* prev(int i) const;
	Pattern pattern() const { return LOCAL; }

	// Variables
	DiversityType type; //!< Enum corresponding to the diversity type (Variety, Majority, Minority)
};

} } // namespace map::detail

#endif
