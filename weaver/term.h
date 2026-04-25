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

struct Variant {
	// Depending on the dialect, this could be one of two things:
	// 1. if the lib is defined for this dialect, then this is an index
	//    into the appropriate lib in Program::libs[meta.dialect]
	// 2. if the lib is not defined for this dialect, then this is a
	//    process definition for that dialect independent of the lib
	std::any index;

	Metadata meta;

	int super; // variant this was derived from
	std::vector<int> derived; // set of derived variants

	Variant(Metadata meta, std::any index=std::any(), int super=-1);
	Variant(std::string dialect, std::any index=std::any(), int super=-1);
	~Variant();

	operator bool() const;

	template <typename T>
	void set(const T &value) {
		index = value;
	}

	template <typename T>
	T &as() {
		return std::any_cast<T&>(index);
	}

	template <typename T>
	const T &as() const {
		return std::any_cast<const T&>(index);
	}
};

// Term represents a function/process definition in the program
// This includes both the declaration and the implementation
struct Term {
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

	int createVariant(Variant var);

	// Prints the term details for debugging
	void print() const;
};

}

