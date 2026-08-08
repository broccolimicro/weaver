#include "program.h"

namespace weaver {

Module::Module() {
	isTech = false;
}

Module::~Module() {
}

int Module::getTerm(Decl decl) {
	vector<int> ids = findTerms(decl);
	if (ids.empty()) {
		ids.push_back(createTerm(decl));
	} else if (ids.size() > 1u) {
		warning("", "ambiguous term names", __FILE__, __LINE__);
	}

	if (not terms[ids[0]].decl.qualified and decl.qualified) {
		terms[ids[0]].decl = decl;
	}

	return ids[0];
}

int Module::createTerm(Term term) {
	terms.push_back(term);
	return (int)terms.size()-1;
}

int Module::createType(Type type) {
	types.push_back(type);
	return (int)types.size()-1;
}

int Module::findType(string name) const {
	for (int i = 0; i < (int)types.size(); i++) {
		if (types[i].name == name) {
			return i;
		}
	}
	return -1;
}

vector<int> Module::findTerms(Decl decl) const {
	vector<int> result;
	for (int i = 0; i < (int)terms.size(); i++) {
		if (terms[i].decl == decl) {
			result.push_back(i);
		}
	}
	return result;
}

void Module::print() const {
	printf("module %s {\n", name.c_str());
	for (int i = 0; i < (int)types.size(); i++) {
		types[i].print();
	}
	for (int i = 0; i < (int)terms.size(); i++) {
		terms[i].print();
	}
	printf("}\n");
}

Program::Program() {
	global = -1;
}

Program::~Program() {
}

int Program::pushModule(string name) {
	mods.push_back(Module());
	mods.back().name = name;
	return (int)mods.size()-1;
}

int Program::findModule(string name) const {
	for (int i = 0; i < (int)mods.size(); i++) {
		if (mods[i].name == name) {
			return i;
		}
	}
	return -1;
}

int Program::getModule(string name) {
	int result = findModule(name);
	if (result < 0) {
		result = pushModule(name);
	}

	return result;
}

TypeId Program::findType(string mod, string name, int index) const {
	if (name.empty()) {
		return TypeId(index, -1);
	}

	if (mod.empty()) {
		// For unqualified names, we have a type lookup priority:
		// 1. First check the global module (for built-in types)
		// 2. Then check the current module (specified by 'index')
		TypeId result;
		if (global >= 0) {
			result.mod = global;
			result.index = mods[global].findType(name);
		}

		if (not result.defined() and index >= 0) {
			result.mod = index;
			result.index = mods[index].findType(name);
		}

		return result;
	}

	// For qualified names, extract the module and look for the type in that
	// module
	index = findModule(mod);
	if (index >= 0) {
		return TypeId(index, mods[index].findType(name));
	}
	return TypeId();
}

Instance Program::findInstance(Typename type, string name, int index) const {
	return Instance(findType(type.mod, type.name, index), name, type.size);
}

Decl Program::findDecl(Prototype proto, int index) const {
	if (proto.qualified and not proto.hashed) {
		proto.hashArgs();
	}

	Decl result;
	if (not proto.mod.empty()) {
		index = findModule(proto.mod);
	}

	result.name = proto.name;
	if (not proto.recv.empty()) {
		result.recv = findType(proto.mod, proto.recv, index);
	} else if (not proto.mod.empty()) {
		result.recv.mod = index;
	}
	result.argsHash = proto.argsHash;
	result.qualified = proto.qualified;
	result.hashed = proto.hashed;
	for (const auto &arg : proto.args) {
		result.args.push_back(findInstance(arg, "", index));
	}
	return result;
}

vector<TermId> Program::findTerms(std::string mod, Decl decl, int index) const {
	if (decl.qualified and not decl.hashed) {
		decl.argsHash = getArgsHash(decl);
		decl.hashed = true;
	}

	vector<TermId> result;
	if (mod.empty()) {
		// Then this is in the global namespace
		if (global >= 0) {
			for (int term : mods[global].findTerms(decl)) {
				result.push_back(TermId(global, term));
			}
		}

		if (result.empty() and index >= 0) {
			for (int term : mods[index].findTerms(decl)) {
				result.push_back(TermId(index, term));
			}
		}
		return result;
	}

	index = findModule(mod);
	if (index >= 0) {
		for (int term : mods[index].findTerms(decl)) {
			result.push_back(TermId(index, term));
		}
	}
	return result;
}

vector<TermId> Program::findTerms(Prototype proto, int index) const {
	vector<TermId> result;
	if (proto.empty()) {
		return result;
	}

	return findTerms(proto.mod, findDecl(proto), index);
}

TermId Program::getTerm(Prototype proto, int index) {
	if (not proto.mod.empty()) {
		index = getModule(proto.mod);
	}

	return getTerm(index, findDecl(proto, index));
}

Typename Program::getTypename(TypeId idx, std::vector<int> size) const {
	Typename result;
	if (typeValid(idx)) {
		result.mod = modAt(idx).name;
		result.name = typeAt(idx).name;
	}
	result.size = size;
	return result;
}

Typename Program::getTypename(const Instance &inst) const {
	return getTypename(inst.type, inst.size);
}

Prototype Program::getPrototype(const Decl &decl, std::string mod) const {
	Prototype result;
	result.mod = mod;
	result.name = decl.name;
	if (typeValid(decl.recv)) {
		result.recv = typeAt(decl.recv).name;
	}
	for (auto i = decl.args.begin(); i != decl.args.end(); i++) {
		result.args.push_back(getTypename(*i));
	}
	result.argsHash = getHash(result.args);
	result.qualified = true;
	result.hashed = true;
	return result; 
}

Prototype Program::getPrototype(TermId idx) const {
	if (not termValid(idx)) {
		return Prototype();
	}
	return getPrototype(termAt(idx).decl, modAt(idx).name);
}

TermId Program::begin() const {
	return next(TermId(0, -1));
}

TermId Program::next(TermId idx) const {
	idx.index++;
	if (idx.mod < (int)mods.size()
		and idx.index >= (int)mods[idx.mod].terms.size()) {
		idx.mod++;
		while (idx.mod < (int)mods.size()
			and mods[idx.mod].terms.empty()) {
			idx.mod++;
		}
		idx.index = 0;
	}
	if (idx.mod >= (int)mods.size()) {
		return TermId();
	}
	return idx;
}

TermId Program::end() const {
	return TermId();
}

bool Program::modValid(TypeId idx) const {
	return idx.mod >= 0 and idx.mod < (int)mods.size();
}

bool Program::typeValid(TypeId idx) const {
	return modValid(idx) and idx.index >= 0 and idx.index < (int)mods[idx.mod].types.size();
}

bool Program::modValid(TermId idx) const {
	return idx.mod >= 0 and idx.mod < (int)mods.size();
}

bool Program::termValid(TermId idx) const {
	return modValid(idx) and idx.index >= 0 and idx.index < (int)mods[idx.mod].terms.size();
}

bool Program::varValid(TermId idx) const {
	return termValid(idx) and idx.var >= 0 and idx.var < (int)mods[idx.mod].terms[idx.index].variants.size();
}

const Module &Program::modAt(TypeId idx) const {
	return mods[idx.mod];
}

const Type &Program::typeAt(TypeId idx) const {
	return mods[idx.mod].types[idx.index];
}

Module &Program::modAt(TypeId idx) {
	return mods[idx.mod];
}

Type &Program::typeAt(TypeId idx) {
	return mods[idx.mod].types[idx.index];
}

const Module &Program::modAt(TermId idx) const {
	return mods[idx.mod];
}

const Term &Program::termAt(TermId idx) const {
	return mods[idx.mod].terms[idx.index];
}

const Variant &Program::varAt(TermId idx) const {
	return mods[idx.mod].terms[idx.index].variants[idx.var];
}

Module &Program::modAt(TermId idx) {
	return mods[idx.mod];
}

Term &Program::termAt(TermId idx) {
	return mods[idx.mod].terms[idx.index];
}

Variant &Program::varAt(TermId idx) {
	return mods[idx.mod].terms[idx.index].variants[idx.var];
}

size_t Program::getArgsHash(Decl decl) const {
	std::vector<Typename> args;
	for (auto i = decl.args.begin(); i != decl.args.end(); i++) {
		args.push_back(getTypename(*i));
	}
	return getHash(args);
}

TermId Program::getTerm(int mod, Decl decl) {
	if (decl.qualified and decl.argsHash == 0) {
		decl.argsHash = getArgsHash(decl);
		decl.hashed = true;
	}
	return TermId(mod, mods[mod].getTerm(decl));
}

TermId Program::createTerm(int mod, Term term) {
	if (term.decl.qualified and term.decl.argsHash == 0) {
		term.decl.argsHash = getArgsHash(term.decl);
		term.decl.hashed = true;
	}
	return TermId(mod, mods[mod].createTerm(term));
}

void Program::print(TermId id) const {
	std::string result = "undef";
	if (termValid(id)) {
		result = getPrototype(id).to_string();
	}
	printf("%s", result.c_str());
}

void Program::print(TypeId id) const {
	std::string result = "void";
	if (typeValid(id)) {
		result = typeAt(id).name;
	}
	printf("%s", result.c_str());
}

void Program::print(const Decl &decl) const {
	if (typeValid(decl.recv)) {
		print(decl.recv);
		printf("::");
	}
	printf("%s(", decl.name.c_str());
	for (int i = 0; i < (int)decl.args.size(); i++) {
		if (i != 0) {
			printf(", ");
		}
		print(decl.args[i]);
	}
	printf(") ");
	print(decl.ret);
	printf(" hash=%zu", decl.argsHash);
}

void Program::print(const Instance &inst) const {
	print(inst.type);
	printf(" %s", inst.name.c_str());
	for (int i = 0; i < (int)inst.size.size(); i++) {
		printf("[%d]", inst.size[i]);
	}
}

void Program::print(const Type &type) const {
	std::string typestr = "invalid";
	if (type.kind == -1) {
		typestr = "interface";
	} else if (type.kind >= 0) {
		typestr = "type";
	}

	// Print type details for debugging
	printf("%s %s {\n", typestr.c_str(), type.name.c_str());
	// Print all data members
	for (int i = 0; i < (int)type.members.size(); i++) {
		printf("instance ");
		print(type.members[i]);
		printf("\n");
	}
	// Print all methods
	for (int i = 0; i < (int)type.methods.size(); i++) {
		printf("decl ");
		print(type.methods[i]);
		printf("\n");
	}
	printf("}\n");
}

void Program::print(const Term &term) const {
	// Print term details for debugging
	printf("term ");
	print(term.decl);
	printf("\n");
	for (int i = 0; i < (int)term.variants.size(); i++) {
		printf("\t%d: ", i);
		term.variants[i].meta.print();
		printf(" -> {");
		for (int j = 0; j < (int)term.variants[i].derived.size(); j++) {
			if (j != 0) {
				printf(" ");
			}
			printf("%d", term.variants[i].derived[j]);
		}
		printf("}\n");
	}
}

void Program::print(const Module &mod) const {
	printf("module %s {\n", mod.name.c_str());
	for (int i = 0; i < (int)mod.types.size(); i++) {
		print(mod.types[i]);
	}
	for (int i = 0; i < (int)mod.terms.size(); i++) {
		print(mod.terms[i]);
	}
	printf("}\n");
}

void Program::print() const {
	for (int i = 0; i < (int)mods.size(); i++) {
		print(mods[i]);
	}
}

void loadGlobalTypes(Program &prgm) {
	if (prgm.global >= 0) {
		return;
	}
	Module glob;
	// The global module contains fundamental interfaces that form
	// the base of the type system. These are kept in the global scope
	// so they are accessible from all modules.
	glob.types.push_back(Type::interfaceOf("wire"));    // Wire interface
	glob.types.push_back(Type::interfaceOf("bool"));    // Boolean interface
	glob.types.push_back(Type::interfaceOf("ufixed"));  // Unsigned fixed-point number interface
	glob.types.push_back(Type::interfaceOf("fixed"));   // Fixed-point number interface
	glob.types.push_back(Type::interfaceOf("chan"));    // Channel interface for communication
	prgm.mods.push_back(glob);
	prgm.global = (int)prgm.mods.size()-1;
}

}
