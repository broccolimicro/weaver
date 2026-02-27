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

Term::Variant::Variant(int super, std::any def, Metadata meta) {
	this->super = super;
	this->def = def;
	this->meta = meta;
}

Term::Variant::~Variant() {
}

Term::Term() {
	// Default constructor creates an empty term
	// kind will be uninitialized, decl empty, symb empty, def empty
}

Term::Term(string name, vector<Instance> args, TypeId ret, TypeId recv) {
	decl = Decl(name, args, ret, recv);  // Initialize the declaration
}

Term::~Term() {
	// No specific cleanup needed
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
	return -1;
}

int Term::getDialect(string name, Dialect::Factory factory) {
	int result = findDialect(name);
	if (result != -1) {
		return result;
	}
	return pushDialect(name, factory);
}

int Term::findVariant(Condition cond) const {
	for (int i = (int)variants.size()-1; i >= 0; i--) {
		if (variants[i].meta.meets(cond)) {
			return i;
		}
	}
	return -1;
}

void Term::print() const {
	// Print term details for debugging
	printf("term ");
	decl.print();
	for (int i = 0; i < (int)variants.size(); i++) {
		printf("\t%d: ", i);
		variants[i].meta.print();
		printf(" -> {");
		for (int j = 0; j < (int)variants[i].derived.size(); j++) {
			if (j != 0) {
				printf(" ");
			}
			printf("%d", variants[i].derived[j]);
		}
		printf("}\n");
	}
}

}
