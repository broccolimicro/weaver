#include "meta.h"
#include "term.h"

namespace weaver {

std::string Metadata::dialect() const {
	if (kind < 0) {
		// this is an interface
		return "";
	}
	return Term::dialects[kind].name;
}

void Metadata::set(std::string name) {
	analysis.insert({name, std::any()});
}

bool Metadata::has(std::string name) const {
	return analysis.find(name) != analysis.end();
}

void Metadata::unset(std::string name) {
	analysis.erase(name);
}

bool Metadata::meets(const Condition &cond) const {
	if (cond.dialect != dialect()) {
		return false;
	}

	for (auto i = cond.analyses.begin(); i != cond.analyses.end(); i++) {
		bool success = true;
		for (auto j = i->begin(); j != i->end() and success; j++) {
			success = has(*j);
		}

		if (success) {
			return true;
		}
	}
	return false;
}

void Metadata::print() const {
	printf("%s {", kind < 0 ? "ctx" : Term::dialects[kind].name.c_str());
	for (auto i = analysis.begin(); i != analysis.end(); i++) {
		if (i != analysis.begin()) {
			printf(" ");
		}
		printf("%s", i->first.c_str());
	}
	printf("}");
}

}
