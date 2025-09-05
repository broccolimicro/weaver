#include "term.h"

namespace weaver {

// Initialize the static dialect registry
vector<Term::Dialect> Term::dialects;

Term::Dialect::Dialect() {
	// Default constructor creates an empty dialect
	name = "";
	factory = nullptr;
}

Term::Dialect::Dialect(string name, Term::Dialect::Factory factory) {
	// Initialize with the specified name and factory function
	this->name = name;
	this->factory = factory;
}

Term::Dialect::~Dialect() {
	// No specific cleanup needed
}

Term::Term() {
	// Default constructor creates an empty term
	// kind will be uninitialized, decl empty, symb empty, def empty
}

Term::~Term() {
	// No specific cleanup needed
}

Term Term::procOf(int kind, string name, vector<Instance> args, TypeId ret, TypeId recv) {
	// Factory method to create a process term
	Term result;
	result.kind = kind;  // Set the kind of process
	result.decl = Decl(name, args, ret, recv);  // Initialize the declaration
	return result;
}

Term Term::contextOf(string name, vector<Instance> args, TypeId ret, TypeId recv) {
	// Factory method to create a context term
	Term result;
	result.kind = Term::CONTEXT;  // Set kind to CONTEXT
	result.decl = Decl(name, args, ret, recv);  // Initialize the declaration
	return result;
}

int Term::pushDialect(string name, Dialect::Factory factory) {
	// Register a new dialect with the given name and factory function
	dialects.push_back(Dialect(name, factory));
	// Return the index of the newly registered dialect
	return (int)dialects.size()-1;
}

int Term::findDialect(string name) {
	// Search for a dialect with the given name
	for (int i = 0; i < (int)dialects.size(); i++) {
		if (dialects[i].name == name) {
			return i;
		}
	}
	// Return NONE if no matching dialect is found
	return Term::NONE;
}

int Term::getDialect(string name, Dialect::Factory factory) {
	int result = findDialect(name);
	if (result != Term::NONE) {
		return result;
	}
	return pushDialect(name, factory);
}

const Term::Dialect &Term::dialect() const {
	return Term::dialects[kind];
}

void Term::print() const {
	// Print term details for debugging
	printf("term %d ", kind);
	decl.print();
}

}
