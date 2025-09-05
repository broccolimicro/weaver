#pragma once

#include "instance.h"
#include "symbol.h"

#include <any>
#include <parse/syntax.h>
#include <parse/tokenizer.h>

namespace weaver {

// Term represents a function/process definition in the program
// This includes both the declaration and the implementation
struct Term {
	Term();
	~Term();

	// The dialect system allows for multiple language representations of
	// term definitions. Each dialect has its own syntax and semantics, but
	// shares the common term declaration structure. This enables support
	// for different hardware description languages within the same framework.
	struct Dialect {
		// Factory function type for creating term implementations from syntax
		typedef std::any (*Factory)(string name, const parse::syntax*, tokenizer*);

		Dialect();
		Dialect(string name, Factory factory);
		~Dialect();
		
		string name;    // Name of the dialect (e.g., "chp", "hse", "prs")
		Factory factory; // Factory function to create dialect-specific term definition
	};

	enum {
		NONE = -2,       // Invalid term kind
		CONTEXT = -1,    // Context term (special term type for handling contexts)
		PROCESS = 0,     // Process term (regular function/process)
	};

	// Global registry of all available dialects
	static vector<Dialect> dialects;
	vector<TermId> impl;

	int kind;           // Kind of term (PROCESS, CONTEXT, or a user-defined kind)
	Decl decl;          // Declaration of the term (name, args, return type, etc.)
	SymbolTable symb;   // Symbol table for local variables in the term
	std::any def;       // Dialect-specific definition of the term

	// Creates a process term with the specified parameters
	static Term procOf(int kind, string name, vector<Instance> args, TypeId ret=TypeId(), TypeId recv=TypeId());
	
	// Creates a context term with the specified parameters
	static Term contextOf(string name, vector<Instance> args, TypeId ret=TypeId(), TypeId recv=TypeId());

	// Registers a new dialect with a name and factory function
	// Returns the index of the newly registered dialect
	static int pushDialect(string name, Dialect::Factory factory);
	
	// Finds a dialect by name
	// Returns the index of the dialect, or NONE if not found
	static int findDialect(string name);

	static int getDialect(string name, Dialect::Factory factory = nullptr);

	const Dialect &dialect() const;

	template <typename T>
	T &as() {
		return std::any_cast<T&>(def);
	}

	template <typename T>
	const T &as() const {
		return std::any_cast<const T&>(def);
	}

	// Prints the term details for debugging
	void print() const;
};

}

