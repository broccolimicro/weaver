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

struct Scope {
	enum {
		NOTFOUND=-1
	};
	vector<Instance> tbl;
	int parent;
	vector<int> child;

	int find(string name) const;
};

struct SymbolTable {
	vector<Scope> scope;
	int curr;

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
