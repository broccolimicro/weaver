#include "project.h"

#include <common/text.h>
#include <filesystem>

namespace weaver {

Filetype::Filetype() {
	read = nullptr;
	load = nullptr;
	write = nullptr;
}

Filetype::Filetype(string dialect, string ext, string build, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write) {
	this->dialect = dialect;
	this->ext = ext;
	this->build = build;
	this->read = read;
	this->load = load;
	this->write = write;
}

Filetype::~Filetype() {
}

Project::Project(fs::path root) {
	if (root.empty()) {
		workDir = fs::current_path();
	} else {
		workDir = root;
	}
	rootDir = workDir;
	while (not rootDir.empty()
		and rootDir.parent_path() != rootDir
		and not fs::exists(rootDir / "lm.mod")) {
		rootDir = rootDir.parent_path();
	}

	includePath.push_back(rootDir / SOURCE);
	includePath.push_back(rootDir / VENDOR);

	char *loom_tech = std::getenv("LOOM_TECH");
	if (loom_tech != nullptr) {
		techDir = fs::path(loom_tech);
	} else {
#if defined(_WIN32) || defined(_WIN64)
		techDir = fs::path("C:\\Program Files (x86)\\Loom\\share\\tech");
#else
		techDir = fs::path("/usr/local/share/tech");
#endif
	}
}

Project::~Project() {
}

int Project::pushFiletype(string dialect, string ext, string build, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write) {
	// Register a new dialect with the given name and factory function
	filetypes.push_back(Filetype(dialect, ext, build, read, load, write));
	// Return the index of the newly registered dialect
	return (int)filetypes.size()-1;
}

const Filetype *Project::getExtension(string ext) const {
	for (int i = 0; i < (int)filetypes.size(); i++) {
		if (filetypes[i].ext == ext) {
			return &filetypes[i];
		}
	}
	return nullptr;
}

const Filetype *Project::getDialect(string dialect) const {
	for (int i = 0; i < (int)filetypes.size(); i++) {
		if (filetypes[i].dialect == dialect) {
			return &filetypes[i];
		}
	}
	return nullptr;
}

bool Project::incl(fs::path path, fs::path from) {
	if (from.empty()) {
		from = workDir;
	}

	fs::path filename;
	if (path.is_absolute()) {
		if (fs::exists(path)) {
			filename = path.string();
		}
	} else {
		if (fs::exists(from / path)) {
			filename = (from / path).string();
		}
		for (auto i = includePath.begin(); i != includePath.end() and filename.empty(); i++) {
			if (fs::exists(*i / path)) {
				filename = (*i / path).string();
			}
		}
	}
	if (filename.empty()) {
		string pathstr = path.string();
		printf("error: file not found '%s'\n", pathstr.c_str());
		return false;
	}
	
	auto pos = find(imports.begin(), imports.end(), filename);
	if (pos == imports.end()) {
		imports.push_back(filename);
	}

	return true;	
}

bool Project::read(Program &prgm, fs::path path) {
	if (path.empty()) {
		return false;
	}

	string ext = path.extension().string().substr(1);
	auto filetype = getExtension(ext);
	if (filetype == nullptr) {
		printf("error: unrecognized filetype '%s'\n", ext.c_str());
		return false;
	}

	fs::path canon = path;
	if (not canon.is_absolute()) {
		canon = workDir / canon;
	}

	sources.push_back(Source());
	sources.back().path = fs::relative(canon, workDir);
	sources.back().modName = pathToModule(canon);
	sources.back().filetype = filetype;
	sources.back().tokens = shared_ptr<tokenizer>(new tokenizer());

	if (filetype->read != nullptr) {
		ifstream fin;
		string pathstr = path.string();
		fin.open(pathstr.c_str(), ios::binary | ios::in);
		if (not fin.is_open()) {
			printf("error: file not found '%s'\n", pathstr.c_str());
			return false;
		}

		fin.seekg(0, ios::end);
		int size = (int)fin.tellg();
		string buffer(size, ' ');
		fin.seekg(0, ios::beg);
		fin.read(&buffer[0], size);
		fin.close();

		filetype->read(*this, sources.back(), buffer);
	}
	return true;
}

bool Project::load(Program &prgm) {
	// TODO(edward.bingham) this is still wrong, we have to create a DAG and walk the DAG backwards from the leaves...
	
	for (int i = 0; i < (int)imports.size(); i++) {
		if (not read(prgm, imports[i])) {
			return false;
		}
	}

	while (not sources.empty()) {
		if (sources.back().filetype->load != nullptr) {
			sources.back().filetype->load(*this, prgm, sources.back());
		}
		sources.pop_back();
	}

	return true;
}

bool Project::save(Program &prgm, int modIdx, int termIdx, int varIdx) {
	const weaver::Module &mod = prgm.mods[modIdx];
	const weaver::Term &term = mod.terms[termIdx];
	const weaver::Variant &variant = term.variants[varIdx];

	auto filetype = getDialect(variant.meta.dialect());
	if (filetype == nullptr or filetype->write == nullptr) {
		return false;
	}

	fs::path emitDir = rootDir / BUILD / rootpathFromModule(mod.name);
	std::filesystem::create_directories(emitDir.string());
	
	string filename = term.decl.name + filetype->ext;	
	filetype->write((emitDir / filename).string(), *this, prgm, modIdx, termIdx, varIdx);
	return true;
}

void Project::save(Program &prgm) {
	for (int i = 0; i < (int)prgm.mods.size(); i++) {
		for (int j = 0; j < (int)prgm.mods[i].terms.size(); j++) {
			for (int k = 0; k < (int)prgm.mods[i].terms[j].variants.size(); k++) {
				if (prgm.mods[i].terms[j].variants[k].meta.kind < 0) {
					printf("internal:%s:%d: dialect not defined for term '%s'\n", __FILE__, __LINE__, prgm.mods[i].terms[j].decl.name.c_str());
					continue;
				}
				save(prgm, i, j, k);
			}
		}
	}
}

void Project::setTech(string cmd) {
	std::vector<std::string> args = splitArguments(cmd);
	if (args.empty()) {
		return;
	}

	std::string name = args[0];
	args.erase(args.begin());

	fs::path path;
	if (name.size() >= 1u and name[0] == '/') {
		path = name;
	} else if (name.size() >= 2u and name[0] == '.' and name[1] == '/') {
		path = workDir / name.substr(2);
	} else if (name.size() >= 2u and name[0] == '/' and name[1] == '/') {
		path = rootDir / name.substr(2);
	} else {
		path = techDir / name;
	}

	fs::path parent = path;
	if (not fs::is_directory(parent)) {
		parent = parent.parent_path();
	}

	if (not fs::exists(path)) {
		string pathstr = path.string();
		printf("tech directory '%s' not found\n", pathstr.c_str());
		printf("the tech directory may be specified using the $LOOM_TECH variable\n");
		return;
	}

	tech.path = path.string();
	tech.args = args;
	if (tech.lib.empty()) {
		tech.lib = (parent / "cells").string();
	}
}

void Project::setTechLib(string path) {
	tech.lib = path;
}

vector<string> Project::listTech() const {
	vector<string> result;
	if (not fs::exists(techDir)) {
		return result;
	}

	for (const auto &entry : fs::directory_iterator(techDir)) {
		if (entry.is_directory() and entry.path().stem().string()[0] != '_') {
			result.push_back(entry.path().stem());
		}
	}
	return result;
}

bool Project::hasMod() const {
	return not rootDir.empty() and rootDir.parent_path() != rootDir;
}

void Project::vendor() const {
}

void Project::tidy() {
}

string Project::pathToModule(fs::path path) const {
	string result;

	string ext = path.extension().string().substr(1);
	auto filetype = getExtension(ext);

	fs::path dirInModule = fs::relative(path.parent_path(), rootDir);
	if (dirInModule.lexically_normal() != ".") {
		result = (fs::path(modName) / dirInModule / path.stem()).string();
	} else {
		result = (fs::path(modName) / path.stem()).string();
	}

	if (filetype != nullptr and not filetype->dialect.empty()) {
		result += ">>" + filetype->dialect;
	}
	return result;
}

fs::path Project::pathFromModule(string mod) const {
	fs::path canon = mod;
	if (canon.extension().empty()) {
		canon = mod + ".wv";
	}
	if (not canon.is_absolute()) {
		if (mod.rfind(fs::relative(workDir, rootDir).string(), 0) == 0) {
			canon = rootDir / canon;
		} else {
			canon = workDir / canon;
		}
	}
	return canon;
}

fs::path Project::relpathFromModule(string mod) const {
	return fs::relative(pathFromModule(mod), workDir);
}

fs::path Project::rootpathFromModule(string mod) const {
	return fs::relative(pathFromModule(mod), rootDir);
}

fs::path Project::buildPath(string dir, string filename) const {
	return rootDir / BUILD / dir / filename;
}

fs::path Project::debugPath(string dir, string filename) const {
	return rootDir / DEBUG / dir / filename;
}

}
