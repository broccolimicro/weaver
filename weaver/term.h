#pragma once

#include "instance.h"
#include "symbol.h"

#include <any>
#include <parse/syntax.h>
#include <parse/tokenizer.h>

namespace weaver {

struct Term {
	Term();
	~Term();

	struct Dialect {
		typedef std::any (*Factory)(const parse::syntax*, tokenizer*);

		Dialect();
		Dialect(string name, Factory factory);
		~Dialect();
		
		string name;
		Factory factory;
	};

	enum {
		NONE = -2,
		CONTEXT = -1,
		PROCESS = 0,
	};

	static vector<Dialect> dialects;

	int kind;
	Decl decl;
	SymbolTable symb;
	std::any def;

	static Term procOf(int kind, string name, vector<Instance> args, TypeId ret=TypeId(), TypeId recv=TypeId());
	static Term contextOf(string name, vector<Instance> args, TypeId ret=TypeId(), TypeId recv=TypeId());

	static int pushDialect(string name, Dialect::Factory factory);
	static int findDialect(string name);

	void print();
};

}

