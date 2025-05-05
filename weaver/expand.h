#pragma once

#include "instance.h"
#include "program.h"

#include <vector>
#include <set>

using std::vector;
using std::set;

namespace weaver {

// VariableExpander traverses nested type hierarchies to flatten them
// This is useful for analyzing complex nested structures or generating
// flattened representations of hierarchical hardware designs
struct VariableExpander {
	VariableExpander(const Program &prgm);
	~VariableExpander();

	// StackElem represents an element in the expansion stack
	struct StackElem {
		Instance name;  // The instance being expanded
		int index;      // Current index in the expansion process
	};

	const Program &prgm;  // Reference to the program containing types

	vector<StackElem> stack;  // Stack for depth-first traversal of nested types
	set<TypeId> registry;     // Set of already registered types to avoid cycles

	// Registers a type to avoid expanding it (prevents infinite recursion for cyclic types)
	void registerType(TypeId type);
	
	// Registers multiple types to avoid expanding them
	void registerTypes(vector<TypeId> types);

	// Implements the depth-first search algorithm to find the next instance to expand
	void search();

	// Iterator interface for traversing all nested instances
	void start(Instance base);
	
	// Gets the next instance in the expansion
	Instance next();
	
	// Checks if the expansion is complete
	bool done() const;
};

}
