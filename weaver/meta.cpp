#include "meta.h"
#include "term.h"

namespace weaver {

Metadata::Metadata(int kind) {
	this->kind = kind;
	this->cost = std::numeric_limits<double>::infinity();
}

Metadata::~Metadata() {
}

std::string Metadata::dialect() const {
	if (kind < 0) {
		// this is an interface
		return "";
	}
	return Term::dialects[kind].name;
}

void Metadata::set(std::string name) {
	props.insert({name, std::any()});
}

bool Metadata::has(std::string name) const {
	return props.find(name) != props.end();
}

void Metadata::unset(std::string name) {
	props.erase(name);
}

void Metadata::print() const {
	printf("%s {", kind < 0 ? "ctx" : Term::dialects[kind].name.c_str());
	for (auto i = props.begin(); i != props.end(); i++) {
		if (i != props.begin()) {
			printf(" ");
		}
		printf("%s", i->first.c_str());
	}
	printf("}");
}

}
