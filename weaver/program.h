#pragma once

#include "term.h"
#include "type.h"
#include "proto.h"

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
	bool isTech;

	Module();
	~Module();

	int getTerm(Decl decl);
	// Creates a new term (function/process) in this module
	// Returns the index of the newly created term
	int createTerm(Term term);
	
	// Creates a new type in this module
	// Returns the index of the newly created type
	int createType(Type type);

	// Finds a type by its name
	// Returns the index of the type, or -1 if not found
	int findType(string name) const;

	// Finds a term by its prototype declaration
	vector<int> findTerms(Decl decl) const;

	// Prints the module contents for debugging
	void print() const;
};

// Program is the top-level container for all modules in a program
struct Program {
	Program();
	~Program();

	vector<Module> mods;  // All modules in the program

	int global;  // Index of the global module containing built-in types

	int pushModule(string name);
	int findModule(string name) const;

	// Creates a new module with the given name
	// Returns the index of the newly created module
	int getModule(string name);

	// Finds a type across all modules by its qualified name
	// First searches in the global module, then in the module at 'index'
	// For qualified names (like "mod.type"), searches in the specified module
	TypeId findType(string mod, string name, int index=-1) const;

	// Converts a typename into an instance
	Instance findInstance(Typename type, string name, int index=-1) const;

	// Converts a prototype into a declaration
	Decl findDecl(Prototype proto, int index=-1) const;

	// Find all terms that match this prototype
	vector<TermId> findTerms(Prototype proto, int index=-1) const;
	TermId getTerm(Prototype proto, int index=-1);

	Typename getTypename(TypeId idx, std::vector<int> size=std::vector<int>()) const;
	Typename getTypename(const Instance &inst) const;
	Prototype getPrototype(const Decl &decl, std::string mod="") const;
	Prototype getPrototype(TermId idx) const;

	TermId begin() const;
	TermId next(TermId idx) const;
	TermId end() const;

	bool modValid(TypeId idx) const;
	bool typeValid(TypeId idx) const;
	bool modValid(TermId idx) const;
	bool termValid(TermId idx) const;
	bool varValid(TermId idx) const;

	// Returns a const reference to the type at the specified TypeId
	const Module &modAt(TypeId idx) const;
	const Type &typeAt(TypeId idx) const;
	
	// Returns a mutable reference to the type at the specified TypeId
	Module &modAt(TypeId idx);
	Type &typeAt(TypeId idx);

	// Returns a const reference to the term at the specified TermId
	const Module &modAt(TermId idx) const;
	const Term &termAt(TermId idx) const;
	const Variant &varAt(TermId idx) const;
	
	// Returns a mutable reference to the term at the specified TermId
	Module &modAt(TermId idx);
	Term &termAt(TermId idx);
	Variant &varAt(TermId idx);

	TermId getTerm(int mod, Decl decl);
	TermId createTerm(int mod, Term term);

	// Prints the program contents for debugging
	void print(TermId id) const;
	void print(TypeId id) const;
	void print(const Decl &decl) const;
	void print(const Instance &inst) const;
	void print(const Type &type) const;
	void print(const Term &term) const;
	void print(const Module &mod) const;
	void print() const;
};

// Initializes the global module with built-in types like chan, fixed, ufixed, bool
void loadGlobalTypes(Program &prgm);

}
