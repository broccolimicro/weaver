#include "term.h"

namespace weaver {

vector<Term::Dialect> Term::dialects;

Term::Dialect::Dialect() {
	name = "";
	factory = nullptr;
}

Term::Dialect::Dialect(string name, Term::Dialect::Factory factory) {
	this->name = name;
	this->factory = factory;
}

Term::Dialect::~Dialect() {
}

Term::Term() {
}

Term::~Term() {
}

Term Term::procOf(int kind, string name, vector<Instance> args, TypeId ret, TypeId recv) {
	Term result;
	result.kind = kind;
	result.decl = Decl(name, args, ret, recv);
	return result;
}

Term Term::contextOf(string name, vector<Instance> args, TypeId ret, TypeId recv) {
	Term result;
	result.kind = Term::CONTEXT;
	result.decl = Decl(name, args, ret, recv);
	return result;
}

int Term::pushDialect(string name, Dialect::Factory factory) {
	dialects.push_back(Dialect(name, factory));
	return (int)dialects.size()-1;
}

int Term::findDialect(string name) {
	for (int i = 0; i < (int)dialects.size(); i++) {
		if (dialects[i].name == name) {
			return i;
		}
	}
	return Term::NONE;
}

void Term::print() {
	printf("term %d ", kind);
	decl.print();
}


}
