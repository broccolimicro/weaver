#include "term.h"

namespace weaver {

Variant::Variant(Metadata meta, std::any def, int super) : meta(meta) {
	this->super = super;
	this->def = def;
	this->fromSource = false;
}

Variant::Variant(std::string dialect, std::any def, int super) : meta(dialect) {
	this->super = super;
	this->def = def;
	this->fromSource = false;
}

Variant::~Variant() {
}

Variant::operator bool() const {
	return def.has_value();
}

Term::Term() {
	// Default constructor creates an empty term
	// variants empty, decl empty, symb empty, index empty
}

Term::Term(Decl decl) {
	this->decl = decl;
}

Term::Term(string name, vector<Instance> args, TypeId ret, TypeId recv) {
	decl = Decl(name, args, ret, recv);  // Initialize the declaration
}

Term::~Term() {
	// No specific cleanup needed
}

int Term::createVariant(Variant var) {
	int result = (int)variants.size();
	variants.push_back(var);
	if (variants.back().super >= 0 and variants.back().super < (int)variants.size()) {
		variants[variants.back().super].derived.push_back(result);
	}
	return result;
}

int Term::rfindVariant(std::string dialect, int from) {
	if (from < 0 or from >= (int)variants.size()) {
		from = (int)variants.size()-1;
	}

	for (; from >= 0; from--) {
		if (variants[from].meta.dialect == dialect) {
			return from;
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
