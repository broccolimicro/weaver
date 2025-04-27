#include "type.h"

namespace weaver {

Type::Type() {
}

Type::~Type() {
}

Type Type::typeOf(string name, vector<Instance> members, vector<Decl> methods) {
	Type result;
	result.kind = TYPE;
	result.name = name;
	result.members = members;
	result.methods = methods;
	return result;
}

Type Type::interfaceOf(string name, vector<Decl> methods) {
	Type result;
	result.kind = INTERFACE;
	result.name = name;
	result.methods = methods;
	return result;
}

void Type::print() {
	printf("type %d %s {\n", kind, name.c_str());
	for (int i = 0; i < (int)members.size(); i++) {
		members[i].print();
	}
	for (int i = 0; i < (int)methods.size(); i++) {
		methods[i].print();
	}
	printf("}\n");
}

}

