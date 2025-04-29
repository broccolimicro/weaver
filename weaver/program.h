#pragma once

#include "term.h"
#include "type.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace weaver {

struct Module {
	string name;
	vector<Term> terms;
	vector<Type> types;
	vector<pair<string, string> > aliases;

	int createTerm(Term term);
	int createType(Type type);

	int findTerm(Decl proto) const;
	int findType(vector<string> name) const;

	void print();
};

struct Program {
	Program();
	~Program();

	vector<Module> mods;
	int global;

	int createModule(string name);

	int findTerm(int index, Decl proto) const;
	TypeId findType(int index, vector<string> name) const;

	const Type &typeAt(TypeId idx) const;
	Type &typeAt(TypeId idx);

	void print();
};

void loadGlobalTypes(Program &prgm);

}
