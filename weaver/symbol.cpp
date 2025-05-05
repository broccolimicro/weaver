#include "symbol.h"

namespace weaver {

Scope::Scope() {
	parent = -1;
}

Scope::~Scope() {
}

int Scope::find(string name) const {
	// Search for an instance with the given name in this scope's symbol table
	for (int i = 0; i < (int)tbl.size(); i++) {
		if (tbl[i].name == name) {
			return i;
		}
	}
	// Return NOTFOUND if no matching instance is found
	return NOTFOUND;
}

SymbolTable::SymbolTable() {
	scope.push_back(Scope());
	curr = 0;
}

SymbolTable::~SymbolTable() {
}

bool SymbolTable::define(Instance inst) {
	if (curr < 0 or curr >= (int)scope.size()) {
		return false;
	}
	// Check for name conflicts only in the current scope
	// This allows shadowing variables in parent scopes
	if (scope[curr].find(inst.name) != Scope::NOTFOUND) {
		return false;
	}

	// Add the instance to the current scope's symbol table
	scope[curr].tbl.push_back(inst);
	return true;
}

void SymbolTable::pushScope() {
	// Create a new scope
	scope.push_back(Scope());
	// Add the new scope as a child of the current scope
	if (curr >= 0 and curr < (int)scope.size()) {
		scope[curr].child.push_back((int)scope.size()-1);
	}
	// Set the parent of the new scope to the current scope
	scope.back().parent = curr;
	// Make the new scope the current scope
	curr = (int)scope.size()-1;
}

void SymbolTable::popScope() {
	// Return to the parent of the current scope
	curr = scope[curr].parent;
}

/*Instance at(Module &types, SymbolTable &symb, vector<int> idx) {
	
}*/

}
