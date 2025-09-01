#include "program.h"

namespace weaver {

int Module::createTerm(Term term) {
	terms.push_back(term);
	return (int)terms.size()-1;
}

int Module::createType(Type type) {
	types.push_back(type);
	return (int)types.size()-1;
}

int Module::findTerm(Decl proto) const {
	for (int i = 0; i < (int)terms.size(); i++) {
		if (terms[i].decl == proto) {
			return i;
		}
	}
	return -1;
}

int Module::findType(vector<string> name) const {
	if (name.size() == 1u) {
		for (int i = 0; i < (int)types.size(); i++) {
			if (types[i].name == name[0]) {
				return i;
			}
		}
	} else {
		// Qualified names not yet implemented
		printf("error: unimplemented\n");
	}
	return -1;
}

void Module::print() {
	printf("module %s {\n", name.c_str());
	for (int i = 0; i < (int)types.size(); i++) {
		types[i].print();
	}
	for (int i = 0; i < (int)terms.size(); i++) {
		terms[i].print();
	}
	printf("}\n");
}

Instance SymbolTable::find(string name) const {
	int i = curr;
	int index = Scope::NOTFOUND;
	while (index == Scope::NOTFOUND and i >= 0) {
		index = scope[i].find(name);
		if (index == Scope::NOTFOUND) {
			i = scope[i].parent;
		}
	}
	if (i >= 0 and index >= 0) {
		return scope[i].tbl[index];
	}
	return Instance();
}

Program::Program() {
	global = -1;
}

Program::~Program() {
}

int Program::pushModule(string name) {
	mods.push_back(Module());
	mods.back().name = name;
	return (int)mods.size()-1;
}

int Program::findModule(string name) {
	for (int i = 0; i < (int)mods.size(); i++) {
		if (mods[i].name == name) {
			return i;
		}
	}
	return -1;
}

int Program::getModule(string name) {
	int result = findModule(name);
	if (result < 0) {
		result = pushModule(name);
	}

	return result;
}

int Program::findTerm(int index, Decl proto) const {
	// Module-qualified names are indicated by a dot separator (e.g., "mod.func")
	size_t dot = proto.name.find_first_of(".");
	if (dot == string::npos) {
		return -1;
	}

	string modName = proto.name.substr(0, dot);
	proto.name = proto.name.substr(dot+1);
	for (int i = 0; i < (int)mods.size(); i++) {
		if (mods[i].name == modName) {
			return mods[i].findTerm(proto);
		}
	}
	return -1;
}

TypeId Program::findType(int index, vector<string> name) const {
	if (name.empty()) {
		return TypeId();
	} else if (name.size() == 1u) {
		// For unqualified names, we have a type lookup priority:
		// 1. First check the global module (for built-in types)
		// 2. Then check the current module (specified by 'index')
		TypeId result;
		if (global >= 0) {
			result.mod = global;
			result.index = mods[global].findType(name);
		}

		if (not result.defined() and index >= 0) {
			result.mod = index;
			result.index = mods[index].findType(name);
		}
		
		return result;
	}

	// For qualified names, extract the module and look for the type in that module
	string modName = name[0];
	name.erase(name.begin());
	for (int i = 0; i < (int)mods.size(); i++) {
		if (mods[i].name == modName) {
			return TypeId(i, mods[i].findType(name));
		}
	}
	return TypeId();
}

TypeId Program::begin() const {
	return next(TypeId(0, -1));
}

TypeId Program::next(TypeId idx) const {
	idx.index++;
	if (idx.mod < (int)mods.size()
		and idx.index >= (int)mods[idx.mod].terms.size()) {
		idx.mod++;
		while (idx.mod < (int)mods.size()
			and mods[idx.mod].terms.empty()) {
			idx.mod++;
		}
		idx.index = 0;
	}
	if (idx.mod >= (int)mods.size()) {
		return TypeId();
	}
	return idx;
}

TypeId Program::end() const {
	return TypeId();
}

const Type &Program::typeAt(TypeId idx) const {
	return mods[idx.mod].types[idx.index];
}

Type &Program::typeAt(TypeId idx) {
	return mods[idx.mod].types[idx.index];
}

void Program::print() {
	for (int i = 0; i < (int)mods.size(); i++) {
		mods[i].print();
	}
}

void loadGlobalTypes(Program &prgm) {
	if (prgm.global >= 0) {
		return;
	}
	Module glob;
	// The global module contains fundamental interfaces that form
	// the base of the type system. These are kept in the global scope
	// so they are accessible from all modules.
	glob.types.push_back(Type::interfaceOf("chan"));    // Channel interface for communication
	glob.types.push_back(Type::interfaceOf("fixed"));   // Fixed-point number interface
	glob.types.push_back(Type::interfaceOf("ufixed"));  // Unsigned fixed-point number interface
	glob.types.push_back(Type::interfaceOf("bool"));    // Boolean interface
	prgm.mods.push_back(glob);
	prgm.global = (int)prgm.mods.size()-1;
}

}
