#pragma once

#include "instance.h"
#include "symbol.h"
#include "meta.h"

#include <string>
#include <vector>
#include <list>
#include <any>
#include <parse/syntax.h>
#include <parse/tokenizer.h>

namespace weaver {

// Term represents a function/process definition in the program
// This includes both the declaration and the implementation
struct Term {
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

	struct Variant {
		std::any def;
		Metadata meta;

		int super; // variant this was derived from
		std::vector<int> derived; // set of derived variants

		template <typename T>
		T &as() {
			return std::any_cast<T&>(def);
		}

		template <typename T>
		const T &as() const {
			return std::any_cast<const T&>(def);
		}
	};

	// Global registry of all available dialects
	static vector<Dialect> dialects;

	// Implements relationships between terms. If one term implements
	// another, then their high-level behaviors should be equivalent within
	// the valid set of environments.
	vector<TermId> impl;

	// TODO(edward.bingham) find a way to think about the valid set of
	// environments. Right now, we just put a random source on every input
	// channel and a sink on every output channel.

	Decl decl;          // Declaration of the term (name, args, return type, etc.)
	SymbolTable symb;   // Symbol table for local variables in the term
	std::vector<Variant> variants; // Dialect-specific definitions of the term

	Term();
	Term(string name, vector<Instance> args, TypeId ret=TypeId(), TypeId recv=TypeId());
	~Term();

	// Registers a new dialect with a name and factory function
	// Returns the index of the newly registered dialect
	static int pushDialect(string name, Dialect::Factory factory);
	
	// Finds a dialect by name
	// Returns the index of the dialect, or NONE if not found
	static int findDialect(string name);

	static int getDialect(string name, Dialect::Factory factory = nullptr);

	int findVariant(Condition cond) const;

	// Prints the term details for debugging
	void print() const;
};

}

