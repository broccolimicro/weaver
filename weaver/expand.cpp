#include "expand.h"

namespace weaver {

VariableExpander::VariableExpander(const Program &prgm) : prgm(prgm) {
	// Initialize with a reference to the program
}

VariableExpander::~VariableExpander() {
	// No specific cleanup needed
}

void VariableExpander::registerType(TypeId type) {
	// Add a type to the registry to avoid expanding it
	registry.insert(type);
}

void VariableExpander::registerTypes(vector<TypeId> types) {
	// Add multiple types to the registry
	registry.insert(types.begin(), types.end());
}

void VariableExpander::search() {
	// This is a depth-first traversal of the type hierarchy
	// that stops when it encounters a registered type
	while (not done()) {
		TypeId type = stack.back().name.type;
		if (stack.back().index < 0) {
			// First visit to this type - check if it's already registered
			if (registry.find(type) != registry.end()) {
				return;
			}
			++stack.back().index;
		} if (stack.back().index >= (int)prgm.typeAt(type).members.size()) {
			// Processed all members of this type - backtrack
			stack.pop_back();
			if (not stack.empty()) {
				++stack.back().index;
			}
		} else {
			// Explore the next member of this type
			stack.push_back({prgm.typeAt(type).members[stack.back().index], -1});
		}
	}
}

void VariableExpander::start(Instance base) {
	// Clear the stack and push the base instance
	stack.clear();
	stack.push_back({base, -1});
	// Start the search
	search();
}

Instance VariableExpander::next() {
	// Save the current element
	Instance elem = stack.back().name;
	elem.name = "";
	for (int i = 0; i < (int)stack.size(); i++) {
		if (i != 0) {
			elem.name += ".";
		}
		elem.name += stack[i].name.name;
	}

	// Move to the next element
	++stack.back().index;
	search();
	// Return the saved element's instance
	return elem;
}

bool VariableExpander::done() const {
	// We're done when the stack is empty
	return stack.empty();
}

}
