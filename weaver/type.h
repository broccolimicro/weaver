#pragma once

#include "instance.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace weaver {

// Type represents a data type or interface definition in the program
struct Type {
	Type();
	~Type();

	enum {
		NONE = -2,        // Invalid type kind
		INTERFACE = -1,   // Interface type (only methods, no data members)
		TYPE = 0,         // Regular data type (has data members and methods)
	};

	int kind;                // Kind of type
	string name;             // Name of the type
	vector<Instance> members; // Data members of the type (empty for interfaces)
	vector<Decl> methods;     // Methods associated with the type

	// Creates a regular data type that can have both data members and methods
	static Type typeOf(string name, vector<Instance> members=vector<Instance>(), vector<Decl> methods=vector<Decl>());
	
	// Creates an interface type that can only have methods (no data members)
	static Type interfaceOf(string name, vector<Decl> methods=vector<Decl>());

	void print() const;
};

}
