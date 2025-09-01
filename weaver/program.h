#pragma once

#include "term.h"
#include "type.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace weaver {

// Module represents a collection of terms (functions/processes) and types
// within the same namespace. It acts like a compilation unit.
struct Module {
	string name;  // Module name that serves as a namespace identifier
	vector<Term> terms;  // Collection of function/process definitions in this module
	vector<Type> types;  // Collection of types defined in this module
	vector<pair<string, string> > aliases;  // Type aliases for this module

	// Creates a new term (function/process) in this module
	// Returns the index of the newly created term
	int createTerm(Term term);
	
	// Creates a new type in this module
	// Returns the index of the newly created type
	int createType(Type type);

	// Finds a term by its prototype declaration
	// Returns the index of the term, or -1 if not found
	int findTerm(Decl proto) const;
	
	// Finds a type by its name
	// Returns the index of the type, or -1 if not found
	int findType(vector<string> name) const;

	// Prints the module contents for debugging
	void print();
};

// Program is the top-level container for all modules in a program
struct Program {
	Program();
	~Program();

	vector<Module> mods;  // All modules in the program
	int global;  // Index of the global module containing built-in types

	int pushModule(string name);
	int findModule(string name);

	// Creates a new module with the given name
	// Returns the index of the newly created module
	int getModule(string name);

	// Finds a term across all modules by its prototype declaration
	// Returns the index of the term, or -1 if not found
	int findTerm(int index, Decl proto) const;
	
	// Finds a type across all modules by its qualified name
	// First searches in the global module, then in the module at 'index'
	// For qualified names (like "mod.type"), searches in the specified module
	TypeId findType(int index, vector<string> name) const;

	TypeId begin() const;
	TypeId next(TypeId idx) const;
	TypeId end() const;

	// Returns a const reference to the type at the specified TypeId
	const Type &typeAt(TypeId idx) const;
	
	// Returns a mutable reference to the type at the specified TypeId
	Type &typeAt(TypeId idx);

	// Prints the program contents for debugging
	void print();
};

// Initializes the global module with built-in types like chan, fixed, ufixed, bool
void loadGlobalTypes(Program &prgm);

}
