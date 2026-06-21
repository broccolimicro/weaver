#include "project.h"

#include <common/text.h>
#include <filesystem>

namespace weaver {

Filetype::Filetype() {
	read = nullptr;
	load = nullptr;
	write = nullptr;
	level = TERM;
}

Filetype::Filetype(string dialect, string ext, string build, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write, ConsolidationLevel level, std::any data) {
	this->dialect = dialect;
	this->ext = ext;
	this->build = build;
	this->read = read;
	this->load = load;
	this->write = write;
	this->data = data;
	this->level = level;
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

int Project::pushFiletype(string dialect, string ext, string build, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write, Filetype::ConsolidationLevel level, std::any data) {
	// Register a new dialect with the given name and factory function
	filetypes.push_back(Filetype(dialect, ext, build, read, load, write, level, data));
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

std::vector<const Filetype*> Project::getDialect(string dialect) const {
	std::vector<const Filetype*> result;
	for (int i = 0; i < (int)filetypes.size(); i++) {
		if (filetypes[i].dialect == dialect) {
			result.push_back(&filetypes[i]);
		}
	}
	return result;
}

bool Project::incl(std::string uri) {
	fs::path path;
	if (uri == modName) {
		path = rootDir / "src";
	} else if (uri.starts_with(modName + "/")) {
		path = rootDir / "src" / uri.substr(modName.size()+1);
	} else {
		path = rootDir / "dep" / uri;
	}

	if (not fs::exists(path)) {
		cout << "error: import not found '" << uri << "'" << endl;
		cout << "note: searched '" << path << "'" << endl;
		return false;
	}

	auto pos = find(imports.begin(), imports.end(), path);
	if (pos == imports.end()) {
		imports.push_back(path);
	}
	return true;
}

bool Project::read(Program &prgm, fs::path path) {
	if (not fs::exists(path)) {
		return false;
	}

	if (fs::is_directory(path)) {
		for (const auto &entry : fs::directory_iterator(path)) {
			if (not fs::is_regular_file(entry.path())) {
				continue;
			}
			string ext = entry.path().extension().string();
			if (not ext.empty()) {
				ext = ext.substr(1);
			}
			auto filetype = getExtension(ext);
			if (filetype != nullptr and not read(prgm, entry.path())) {
				return false;
			}
		}
		return true;
	}

	string ext = path.extension().string();
	if (not ext.empty()) {
		ext = ext.substr(1);
	}
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
			cout << "error: failed to read '" << imports[i] << "'" << endl;
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

	auto filetypes = getDialect(variant.meta.dialect);
	const Filetype *filetype = nullptr;
	for (int i = 0; i < (int)filetypes.size(); i++) {
		if (filetypes[i]->write != nullptr) {
			filetype = filetypes[i];
			break;
		}
	}

	if (filetype == nullptr or filetype->write == nullptr) {
		return false;
	}

	if (filetype->level == Filetype::PROJECT) {
		fs::path emitDir = rootDir / BUILD / rootpathFromModule(modName);
		std::filesystem::create_directories(emitDir.string());
		
		string filename = "project." + filetype->ext;
		filetype->write((emitDir / filename).string(), *this, *filetype, prgm, modIdx, termIdx, varIdx);
	} else if (filetype->level == Filetype::MODULE) {
		fs::path emitDir = rootDir / BUILD / rootpathFromModule(mod.name);
		std::filesystem::create_directories(emitDir.string());
		
		string filename = "module." + filetype->ext;
		filetype->write((emitDir / filename).string(), *this, *filetype, prgm, modIdx, termIdx, varIdx);

	} else {
		fs::path emitDir = rootDir / BUILD / rootpathFromModule(mod.name);
		std::filesystem::create_directories(emitDir.string());

		string filename = prgm.mangleName({modIdx, termIdx, varIdx}) + "." + filetype->ext;
		filetype->write((emitDir / filename).string(), *this, *filetype, prgm, modIdx, termIdx, varIdx);
	}
	return true;
}

void Project::save(Program &prgm, int modIdx, int termIdx) {
	if (prgm.mods[modIdx].terms[termIdx].variants.empty()) {
		return;
	}

	auto &term = prgm.mods[modIdx].terms[termIdx];

	std::vector<int> stack(1, 0);
	while (not stack.empty()) {
		int curr = stack.back();
		stack.pop_back();

		auto &var = term.variants[curr];

		bool doExport = true;
		for (int next : var.derived) {
			if (next < 0 or next >= (int)term.variants.size()) {
				continue;
			}

			stack.push_back(next);
			if (var.meta.dialect == term.variants[next].meta.dialect) {
				doExport = false;
			}
		}

		if (var.meta.dialect.empty()) {
			printf("internal:%s:%d: dialect not defined for term '%s'\n", __FILE__, __LINE__, prgm.mods[modIdx].terms[termIdx].decl.name.c_str());
			continue;
		}

		if (doExport) {
			save(prgm, modIdx, termIdx, curr);
		}
	}
}

void Project::save(Program &prgm) {
	for (int i = 0; i < (int)prgm.mods.size(); i++) {
		for (int j = 0; j < (int)prgm.mods[i].terms.size(); j++) {
			if (not prgm.mods[i].isTech) {
				save(prgm, i, j);
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
		name = fs::relative(path, rootDir).lexically_normal().string();
	} else if (name.size() >= 2u and name[0] == '.' and name[1] == '/') {
		path = workDir / name.substr(2);
		name = fs::relative(path, rootDir).lexically_normal().string();
	} else if (name.size() >= 2u and name[0] == '/' and name[1] == '/') {
		path = rootDir / name.substr(2);
		name = fs::relative(path, rootDir).lexically_normal().string();
	} else {
		path = techDir / name;
		name = fs::relative(path, techDir).lexically_normal().string();
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

	tech.name = name;
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

string topDir(fs::path path) {
	if (path.begin() == path.end()) {
		return "";
	}
	return path.begin()->string();
}

fs::path popTopDir(fs::path path) {
	fs::path result;
	if (path.begin() == path.end()) {
		return result;
	}
	for (auto i = std::next(path.begin()); i != path.end(); i++) {
		result /= *i;
	}
	return result;
}

string Project::pathToModule(fs::path path) const {
	if (not fs::is_directory(path)) {
		path = path.parent_path();
	}

	fs::path dirInModule = fs::relative(path, rootDir).lexically_normal();
	std::string top = topDir(dirInModule);
	if (top == ".") {
		return fs::path(modName).string();
	} else if (top == "src") {
		dirInModule = popTopDir(dirInModule);
		if (dirInModule.empty()) {
			return fs::path(modName).string();
		}
		return (fs::path(modName) / dirInModule).string();
	} else if (top == "dep") {
		// TODO(edward.bingham) parse the lm.mod file in the vendor directory
		return popTopDir(dirInModule);
	}
	return "";
}

fs::path Project::pathFromModule(string mod) const {
	if (mod == modName) {
		return rootDir / "src";
	} else if (mod.rfind(modName+"/", 0) == 0) {
		return rootDir / "src" / mod.substr(modName.size()+1);
	}
	return rootDir / "dep" / mod;
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
