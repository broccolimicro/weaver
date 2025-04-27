#include "symbol.h"

namespace weaver {

int Scope::find(string name) const {
	for (int i = 0; i < (int)tbl.size(); i++) {
		if (tbl[i].name == name) {
			return i;
		}
	}
	return NOTFOUND;
}

bool SymbolTable::define(Instance inst) {
	if (scope[curr].find(inst.name) != Scope::NOTFOUND) {
		return false;
	}

	scope[curr].tbl.push_back(inst);
	return true;
}

void SymbolTable::pushScope() {
	scope.push_back(Scope());
	scope[curr].child.push_back((int)scope.size()-1);
	scope.back().parent = curr;
	curr = (int)scope.size()-1;
}

void SymbolTable::popScope() {
	curr = scope[curr].parent;
}

/*Instance at(Module &types, SymbolTable &symb, vector<int> idx) {
	
}*/

}
