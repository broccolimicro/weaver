#pragma once

#include "instance.h"
#include "program.h"

#include <vector>
#include <set>

using std::vector;
using std::set;

namespace weaver {

struct VariableExpander {
	VariableExpander(const Program &prgm);
	~VariableExpander();

	struct StackElem {
		Instance name;
		int index;
	};

	const Program &prgm;

	vector<StackElem> stack;
	set<TypeId> registry;

	void registerType(TypeId type);
	void registerTypes(vector<TypeId> types);

	void search();

	void start(Instance base);
	Instance next();
	bool done() const;
};

}
