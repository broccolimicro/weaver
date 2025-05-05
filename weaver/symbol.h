#pragma once

#include "instance.h"

#include <vector>
#include <string>
#include <any>

#include <common/interface.h>
#include <parse/syntax.h>
#include <parse/tokenizer.h>

using std::vector;
using std::string;
using std::any;

namespace weaver {

// TODO(edward.bingham) Syntax directed translation from CHP to val-rdy
// dataflow blocks to map to FPGA Golang runtime with thread bool for parallel
// routines, compile to software MLIR? LLVM?
//
// LLVM represents programs as a combined control-flow data-flow graph with scope
// implemented by subgraphs I'm currently representing CHP as a petri-net. Primary
// difference is the flattening of scope.

// Scope represents a single scope level for variable declarations
struct Scope {
	Scope();
	~Scope();

	enum {
		NOTFOUND=-1  // Value returned when a symbol is not found
	};
	vector<Instance> tbl;  // Symbol table for this scope
	int parent;            // Index of the parent scope
	vector<int> child;     // Indices of child scopes

	int find(string name) const;
};

// SymbolTable manages the hierarchy of scopes for variable lookup
// It maintains a tree structure of scopes with parent-child relationships
// and tracks the "current" scope for variable definitions and lookups
struct SymbolTable {
	SymbolTable();
	~SymbolTable();

	vector<Scope> scope;  // All scopes in the hierarchy
	int curr;             // Index of the current scope

	// Returns false if the name is already defined in the current scope
	bool define(Instance inst);
	
	void pushScope();
	void popScope();
	
	Instance find(string name) const;
};

// Variables in Weaver are compound (type, interface)
// Variables in flow are channels (type)
// Variables in chp are data (fixed, float, bool)
// Variables in hse and prs are wires (bool)

// How do I map variables in chp back to weaver?
// Weaver should have some structure to keep track of instances. but does scope matter? Scope only matters to avoid overlap of variable names, so only matters during interpretation.


/*Instance at(Module &types, SymbolTable &symb, vector<int> idx);

struct Scope {
	vector<SymbolTable> scope;

	array<int, 2> find(string name) const;
};*/

}
