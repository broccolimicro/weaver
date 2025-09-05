#include "type.h"

namespace weaver {

Type::Type() {
	// Default constructor creates an empty type
	// kind will be uninitialized, name empty, members empty, methods empty
}

Type::~Type() {
	// No specific cleanup needed
}

Type Type::typeOf(string name, vector<Instance> members, vector<Decl> methods) {
	// Factory method to create a regular data type
	Type result;
	result.kind = TYPE;        // Set kind to regular data TYPE
	result.name = name;        // Set the type name
	result.members = members;  // Set the data members
	result.methods = methods;  // Set the methods
	return result;
}

Type Type::interfaceOf(string name, vector<Decl> methods) {
	// Factory method to create an interface type
	Type result;
	result.kind = INTERFACE;   // Set kind to INTERFACE
	result.name = name;        // Set the interface name
	result.methods = methods;  // Set the methods
	// Note: Members remain empty for interfaces
	return result;
}

void Type::print() const {
	// Print type details for debugging
	printf("type %d %s {\n", kind, name.c_str());
	// Print all data members
	for (int i = 0; i < (int)members.size(); i++) {
		members[i].print();
	}
	// Print all methods
	for (int i = 0; i < (int)methods.size(); i++) {
		methods[i].print();
	}
	printf("}\n");
}

}

