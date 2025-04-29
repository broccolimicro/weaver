#include "expand.h"

namespace weaver {

VariableExpander::VariableExpander(const Program &prgm) : prgm(prgm) {
}

VariableExpander::~VariableExpander() {
}

void VariableExpander::registerType(TypeId type) {
	registry.insert(type);
}

void VariableExpander::registerTypes(vector<TypeId> types) {
	registry.insert(types.begin(), types.end());
}

void VariableExpander::search() {
	while (not done()) {
		TypeId type = stack.back().name.type;
		if (stack.back().index < 0) {
			if (registry.find(type) != registry.end()) {
				return;
			}
			++stack.back().index;
		} if (stack.back().index >= (int)prgm.typeAt(type).members.size()) {
			stack.pop_back();
			if (not stack.empty()) {
				++stack.back().index;
			}
		} else {
			stack.push_back({prgm.typeAt(type).members[stack.back().index], -1});
		}
	}
}

void VariableExpander::start(Instance base) {
	stack.clear();
	stack.push_back({base, -1});
	search();
}

Instance VariableExpander::next() {
	StackElem elem = stack.back();
	++stack.back().index;
	search();
	return elem.name;
}

bool VariableExpander::done() const {
	return stack.empty();
}

}
