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
		printf("error: unimplemented\n");
		// function call?
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
	int index;
	do {
		index = scope[i].find(name);
	} while (index == Scope::NOTFOUND and i >= 0);

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

int Program::createModule(string name) {
	mods.push_back(Module());
	mods.back().name = name;
	return (int)mods.size()-1;
}

int Program::findTerm(int index, Decl proto) const {
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

	string modName = name[0];
	name.erase(name.begin());
	for (int i = 0; i < (int)mods.size(); i++) {
		if (mods[i].name == modName) {
			return TypeId(i, mods[i].findType(name));
		}
	}
	return TypeId();
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
	glob.types.push_back(Type::interfaceOf("chan"));
	glob.types.push_back(Type::interfaceOf("fixed"));
	glob.types.push_back(Type::interfaceOf("ufixed"));
	glob.types.push_back(Type::interfaceOf("bool"));
	prgm.mods.push_back(glob);
	prgm.global = (int)prgm.mods.size()-1;
}

}
