#include "instance.h"

namespace weaver {

TypeId::TypeId() {
	mod = -1;
	index = -1;
}

TypeId::TypeId(int mod, int index) {
	this->mod = mod;
	this->index = index;
}

TypeId::~TypeId() {
}

bool TypeId::defined() const {
	return mod >= 0 and index >= 0;
}

bool operator==(TypeId t0, TypeId t1) {
	return t0.mod == t1.mod and t0.index == t1.index;
}

bool operator!=(TypeId t0, TypeId t1) {
	return t0.mod != t1.mod or t0.index != t1.index;
}

bool operator<(TypeId t0, TypeId t1) {
	return t0.mod < t1.mod or (t0.mod == t1.mod and t0.index < t1.index);
}

bool operator>(TypeId t0, TypeId t1) {
	return t0.mod > t1.mod or (t0.mod == t1.mod and t0.index > t1.index);
}

bool operator<=(TypeId t0, TypeId t1) {
	return t0.mod < t1.mod or (t0.mod == t1.mod and t0.index <= t1.index);
}

bool operator>=(TypeId t0, TypeId t1) {
	return t0.mod > t1.mod or (t0.mod == t1.mod and t0.index >= t1.index);
}

Instance::Instance() {
}

Instance::Instance(TypeId type, string name, vector<int> size) {
	this->type = type;
	this->name = name;
	this->size = size;
}

Instance::~Instance() {
}

void Instance::print() const {
	printf("instance (%d,%d) %s {", type.mod, type.index, name.c_str());
	for (int i = 0; i < (int)size.size(); i++) {
		printf("%d ", size[i]);
	}
	printf("}\n");
}

bool operator==(const Instance &i0, const Instance &i1) {
	// Note: We don't compare names - two instances with different names
	// but the same type and dimensions are considered equal. This is 
	// semantic equality rather than syntactic equality.
	if (i0.type != i1.type or i0.size.size() != i1.size.size()) {
		return false;
	}
	
	for (int i = 0; i < (int)i0.size.size(); i++) {
		if (i0.size[i] != i1.size[i]) {
			return false;
		}
	}
	
	return true;
}

TermId::TermId() {
	mod = -1;
	index = -1;
}

TermId::TermId(int mod, int index) {
	this->mod = mod;
	this->index = index;
}

TermId::~TermId() {
}

bool TermId::defined() const {
	return mod >= 0 and index >= 0;
}

bool operator==(TermId t0, TermId t1) {
	return t0.mod == t1.mod and t0.index == t1.index;
}

bool operator!=(TermId t0, TermId t1) {
	return t0.mod != t1.mod or t0.index != t1.index;
}

bool operator<(TermId t0, TermId t1) {
	return t0.mod < t1.mod or (t0.mod == t1.mod and t0.index < t1.index);
}

bool operator>(TermId t0, TermId t1) {
	return t0.mod > t1.mod or (t0.mod == t1.mod and t0.index > t1.index);
}

bool operator<=(TermId t0, TermId t1) {
	return t0.mod < t1.mod or (t0.mod == t1.mod and t0.index <= t1.index);
}

bool operator>=(TermId t0, TermId t1) {
	return t0.mod > t1.mod or (t0.mod == t1.mod and t0.index >= t1.index);
}

Decl::Decl() {
}

Decl::Decl(string name, vector<Instance> args, TypeId ret, TypeId recv) {
	this->name = name;
	this->args = args;
	this->ret = ret;
	this->recv = recv;
}

Decl::~Decl() {
}

void Decl::print() const {
	printf("decl (%d,%d) %s {\n", recv.mod, recv.index, name.c_str());
	for (int i = 0; i < (int)args.size(); i++) {
		args[i].print();
	}
	printf("} (%d,%d)\n", ret.mod, ret.index);
}

bool operator==(const Decl &d0, const Decl &d1) {
	if (d0.recv != d1.recv or d0.name != d1.name or d0.ret != d1.ret or d0.args.size() != d1.args.size()) {
		return false;
	}

	for (int i = 0; i < (int)d0.args.size(); i++) {
		if (not (d0.args[i] == d1.args[i])) {
			return false;
		}
	}

	return true;
}

}
