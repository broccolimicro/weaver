#pragma once

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace weaver {

// TypeId is a reference to a specific type within a specific module
// Used to uniquely identify a type across the entire program
struct TypeId {
	TypeId();
	TypeId(int mod, int index);
	~TypeId();

	int mod;    // Index of the module containing the type
	int index;  // Index of the type within the module

	// Checks if this TypeId references a valid type
	bool defined() const;
};

// Comparison operators for TypeId
bool operator==(TypeId t0, TypeId t1);
bool operator!=(TypeId t0, TypeId t1);
bool operator<(TypeId t0, TypeId t1);
bool operator>(TypeId t0, TypeId t1);
bool operator<=(TypeId t0, TypeId t1);
bool operator>=(TypeId t0, TypeId t1);

// Instance represents a variable declaration
// This includes the type information and name, but not the value
struct Instance {
	Instance();
	Instance(TypeId type, string name, vector<int> size=vector<int>());
	~Instance();
	
	TypeId type;      // Type of the instance
	string name;      // Name of the instance
	vector<int> size; // Array dimensions, if any

	// Prints the instance details for debugging
	void print() const;
};

// Equality comparison for instances - note that name is NOT considered
// for equality, only type and dimensions (semantic equality, not syntactic)
bool operator==(const Instance &i0, const Instance &i1);

// TermId is a reference to a specific term within a specific module
// Used to uniquely identify a term across the entire program
struct TermId {
	TermId(int mod=-1, int index=-1, int var=-1);
	~TermId();

	int mod;    // Index of the module containing the term
	int index;  // Index of the term within the module
	int var;    // index of the variant within the term

	// Checks if this TermId references a valid term
	bool hasMod() const;
	bool hasTerm() const;
	bool hasVar() const;
};

// Comparison operators for TermId
bool operator==(TermId t0, TermId t1);
bool operator!=(TermId t0, TermId t1);
bool operator<(TermId t0, TermId t1);
bool operator>(TermId t0, TermId t1);
bool operator<=(TermId t0, TermId t1);
bool operator>=(TermId t0, TermId t1);

//     wires, data      devices, behaviors
// interface -> type, context -> func, struct

// Decl represents a function/method prototype
// Used for declaring functions, methods, and processes
struct Decl {
	Decl();
	Decl(string name, vector<Instance> args, TypeId ret=TypeId(), TypeId recv=TypeId());
	~Decl();

	TypeId recv;           // Receiver type for methods (similar to 'this' in C++)
	string name;           // Name of the function/method/process
	vector<Instance> args; // Arguments to the function/method/process
	TypeId ret;            // Return type, or invalid TypeId if none

	// Prints the declaration details for debugging
	void print() const;
};

// Equality comparison for declarations
bool operator==(const Decl &d0, const Decl &d1);

}

