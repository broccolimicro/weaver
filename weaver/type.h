#pragma once

#include "instance.h"

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace weaver {

struct Type {
	Type();
	~Type();

	enum {
		NONE = -2,
		INTERFACE = -1,
		TYPE = 0,
	};

	int kind;
	string name;
	vector<Instance> members;
	vector<Decl> methods;

	static Type typeOf(string name, vector<Instance> members=vector<Instance>(), vector<Decl> methods=vector<Decl>());
	static Type interfaceOf(string name, vector<Decl> methods=vector<Decl>());

	void print();
};

}
