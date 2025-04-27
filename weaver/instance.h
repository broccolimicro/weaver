#pragma once

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace weaver {

struct TypeId {
	TypeId();
	TypeId(int mod, int index);
	~TypeId();

	int mod;
	int index;

	bool defined() const;
};

bool operator==(TypeId t0, TypeId t1);
bool operator!=(TypeId t0, TypeId t1);

// represents variable declarations, this does not include "connections" for functions
struct Instance {
	Instance();
	Instance(TypeId type, string name, vector<int> size=vector<int>());
	~Instance();
	
	TypeId type;
	string name;
	vector<int> size;

	void print();
};

bool operator==(const Instance &i0, const Instance &i1);

//     wires, data      devices, behaviors
// interface -> type, context -> func, struct

// represents function prototypes
struct Decl {
	Decl();
	Decl(string name, vector<Instance> args, TypeId ret=TypeId(), TypeId recv=TypeId());
	~Decl();

	TypeId recv;
	string name;
	vector<Instance> args;
	TypeId ret;

	void print();
};

bool operator==(const Decl &d0, const Decl &d1);

}

