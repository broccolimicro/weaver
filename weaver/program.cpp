#include "program.h"

namespace weaver {

int Module::getTerm(Decl decl) {
	vector<int> ids = findTerms(decl);
	if (ids.empty()) {
		ids.push_back(createTerm(decl));
	}
	if (ids.size() > 1) {
		warning("", "ambiguous term names", __FILE__, __LINE__);
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
		if (terms[i].decl.recv == decl.recv
			and terms[i].decl.name == decl.name
			and terms[i].decl.args.size() == decl.args.size()) {
			bool found = true;
			for (int j = 0; j < (int)decl.args.size() and found; j++) {
				found = (decl.args[j].type == terms[i].decl.args[j].type);
			}
			if (found) {
				result.push_back(i);
			}
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
		return TypeId();
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
	Decl result;
	result.name = proto.name;
	result.recv = findType(proto.mod, proto.recv, index);
	for (auto i = proto.args.begin(); i != proto.args.end(); i++) {
		result.args.push_back(findInstance(*i, "", index));
	}
	return result;
}

vector<TermId> Program::findTerms(Prototype proto, int index) const {
	vector<TermId> result;
	if (proto.empty()) {
		return result;
	}

	if (proto.mod.empty()) {
		// Then this is in the global namespace
		if (global >= 0) {
			for (int term : mods[global].findTerms(findDecl(proto))) {
				result.push_back(TermId(global, term));
			}
		}

		if (result.empty() and index >= 0) {
			for (int term : mods[index].findTerms(findDecl(proto))) {
				result.push_back(TermId(index, term));
			}
		}
		return result;
	}

	index = findModule(proto.mod);
	if (index >= 0) {
		for (int term : mods[index].findTerms(findDecl(proto))) {
			result.push_back(TermId(index, term));
		}
	}
	return result;
}

TermId Program::getTerm(Prototype proto, int index) {
	vector<weaver::TermId> ids = findTerms(proto, index);
		
	if (ids.empty()) {
		int mod = getModule(proto.mod);
		vector<weaver::Instance> args;
		weaver::TypeId recv;
		if (not proto.unqualified) {
			// TODO(edward.bingham) add variable names by looking at ports
			for (auto arg = proto.args.begin(); arg != proto.args.end(); arg++) {
				args.push_back(findInstance(*arg, "", mod));
			}
			recv = findType("", proto.recv, mod);
		}

		int idx = mods[mod].createTerm(weaver::Term(proto.name, args, weaver::TypeId(), recv));
		ids.push_back(weaver::TermId(mod, idx));
	}

	if (ids.size() > 1u) {
		error("", "ambiguous process names", __FILE__, __LINE__);
	}
	return ids[0];
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
	result.unqualified = false;
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

TermId Program::getTerm(int mod, Decl decl) {
	return TermId(mod, mods[mod].getTerm(decl));
}

TermId Program::createTerm(int mod, Term term) {
	return TermId(mod, mods[mod].createTerm(term));
}

void Program::print() const {
	for (int i = 0; i < (int)mods.size(); i++) {
		mods[i].print();
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
