#include "project.h"

#include <common/message.h>
#include <common/text.h>
#include <filesystem>
#include <set>

namespace weaver {

Filetype::Filetype() {
	read = nullptr;
	load = nullptr;
	write = nullptr;
	level = TERM;
}

Filetype::Filetype(string dialect, string ext, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write, ConsolidationLevel level) {
	this->dialect = dialect;
	this->ext = ext;
	this->read = read;
	this->load = load;
	this->write = write;
	this->level = level;
}

Filetype::~Filetype() {
}

Dialect::Dialect(std::string name, parse::factory parse, Load load, Link link, std::any data) : parse(parse) {
	this->name = name;
	this->load = load;
	this->link = link;
	this->parse.data = data;
}

Dialect::~Dialect() {
	load = nullptr;
	link = nullptr;
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

	if (not hasMod()) {
		rootDir = workDir;
	}

	includePath.push_back(rootDir / SOURCE);
	includePath.push_back(rootDir / VENDOR);
	includePath.push_back(rootDir / BUILD);

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

bool Project::pushDialect(string dialect, parse::factory parse, Dialect::Load load, Dialect::Link link, std::any data) {
	return dialects.insert({dialect, Dialect(dialect, parse, load, link, data)}).second;
}

bool Project::pushFiletype(string dialect, string ext, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write, Filetype::ConsolidationLevel level) {
	// Register a new dialect with the given name and factory function
	auto result = filetypes.insert({ext, Filetype(dialect, ext, read, load, write, level)});
	if (not result.second) {
		return false;
	}
	Dialect *ref = getDialect(dialect);
	if (ref != nullptr) {
		ref->filetypes.push_back(ext);
	}
	return true;
}

const Filetype *Project::getFiletype(string ext) const {
	auto pos = filetypes.find(ext);
	if (pos == filetypes.end()) {
		return nullptr;
	}
	return &pos->second;
}

Filetype *Project::getFiletype(string ext) {
	auto pos = filetypes.find(ext);
	if (pos == filetypes.end()) {
		return nullptr;
	}
	return &pos->second;
}

const Dialect *Project::getDialect(string dialect) const {
	auto pos = dialects.find(dialect);
	if (pos == dialects.end()) {
		return nullptr;
	}
	return &pos->second;
}

Dialect *Project::getDialect(string dialect) {
	auto pos = dialects.find(dialect);
	if (pos == dialects.end()) {
		return nullptr;
	}
	return &pos->second;
}

const parse::factory *Project::getParser(string dialect) const {
	auto ref = getDialect(dialect);
	if (ref == nullptr) {
		return nullptr;
	}
	return &ref->parse;
}

std::vector<std::string> Project::getParserIndex() const {
	std::vector<std::string> result;
	for (auto i = dialects.begin(); i != dialects.end(); i++) {
		if (not i->second.parse.empty()) {
			result.push_back(i->first);
		}
	}
	return result;
}

bool Project::inclFile(std::string uri) {
	fs::path path = uri;
	if (path.empty()) {
		path = workDir;
	} else if (not path.is_absolute()) {
		path = workDir / path;
	}

	// If it's in the source or vendor directories, we should treat it like a
	// normal import of the project
	fs::path srcPath = path.lexically_relative(rootDir / SOURCE);
	if (not srcPath.is_absolute() and srcPath.begin()->string() != "..") {
		std::string uri = modName;
		if (not srcPath.empty() and srcPath != ".") {
			uri = (fs::path(modName) / srcPath).string();
		}

		return incl(uri);
	}

	fs::path vendorPath = path.lexically_relative(rootDir / VENDOR);
	if (not vendorPath.is_absolute() and not vendorPath.empty()
		and vendorPath != "." and vendorPath.begin()->string() != "..") {
		return incl(vendorPath.string());
	}

	// otherwise, just directly import the absolute path
	if (not fs::exists(path)) {
		error("", "import not found '" + uri + "'", __FILE__, __LINE__);
		return false;
	}
	if (find(imports.begin(), imports.end(), path) == imports.end()) {
		imports.push_back(path);
	}
	return true;
}

bool Project::incl(std::string uri) {
	fs::path path;
	if (uri == modName) {
		path = SOURCE;
	} else if (uri.starts_with(modName+"/")) {
		path = fs::path(SOURCE) / uri.substr(modName.size()+1);
	} else {
		path = fs::path(VENDOR) / uri;
	}

	// then include the source path 
	fs::path srcPath = rootDir / path;
	if (fs::exists(srcPath)) {
		if (find(imports.begin(), imports.end(), srcPath) == imports.end()) {
			imports.push_back(srcPath);
		}
		return true;
	}

	error("", "import not found '" + uri + "'", __FILE__, __LINE__);
	note("", "searched '" + srcPath.string() + "'", __FILE__, __LINE__);
	return false;
}

bool Project::read(Program &prgm, fs::path path, bool isSource) {
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
			auto filetype = getFiletype(ext);
			if (filetype != nullptr and not read(prgm, entry.path(), isSource)) {
				return false;
			}
		}
		return true;
	}

	string ext = path.extension().string();
	if (not ext.empty()) {
		ext = ext.substr(1);
	}
	auto filetype = getFiletype(ext);
	if (filetype == nullptr) {
		error("", "unrecognized filetype '" + ext + "'", __FILE__, __LINE__);
		return false;
	}

	sources.push_back(Source());
	sources.back().path = fs::relative(path, workDir);
	sources.back().isSource = isSource;
	sources.back().modName = pathToModule(path);
	sources.back().filetype = filetype;
	sources.back().tokens = shared_ptr<tokenizer>(new tokenizer());

	if (filetype->read != nullptr) {
		ifstream fin;
		string pathstr = path.string();
		fin.open(pathstr.c_str(), ios::binary | ios::in);
		if (not fin.is_open()) {
			error("", "file not found '" + pathstr + "'", __FILE__, __LINE__);
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
		if (not read(prgm, imports[i], true)) {
			error("", "failed to read '" + imports[i].string() + "'", __FILE__, __LINE__);
			return false;
		}
	}

	std::set<fs::path> buildImports;
	while (not sources.empty()) {
		std::vector<weaver::TermId> loaded;
		if (sources.back().filetype->load != nullptr) {
			loaded = sources.back().filetype->load(*this, prgm, sources.back());
		}
		sources.pop_back();

		for (auto i : loaded) {
			Prototype proto = prgm.getPrototype(i);
			std::string mangle = proto.mangle(false);

			fs::path modPath = rootDir / BUILD / rootpathFromModule(prgm.mods[i.mod].name);
			fs::path projPath = rootDir / BUILD / rootpathFromModule(modName);
			for (const auto &ext : filetypes) {
				if (ext.second.load == nullptr) {
					continue;
				}

				fs::path searchPath;
				if (ext.second.level == Filetype::TERM) {
					searchPath = modPath / (mangle + "." + ext.first);
				} else if (ext.second.level == Filetype::MODULE) {
					searchPath = modPath / ("module." + ext.first);
				} else if (ext.second.level == Filetype::PROJECT) {
					searchPath = projPath / ("project." + ext.first);
				} else {
					continue;
				}

				if (fs::exists(searchPath)) {
					if (buildImports.insert(searchPath).second) {
						if (not read(prgm, searchPath, false)) {
							warning("", "failed to read '" + searchPath.string() + "'", __FILE__, __LINE__);
						}
					}
				}
			}
		}
	}

	return true;
}

bool Project::save(Program &prgm, TermId id) {
	if (id.hasVar()) {
		const weaver::Variant &variant = prgm.varAt(id);
		if (variant.fromSource) {
			return false;
		}

		const Dialect *dialect = getDialect(variant.meta.dialect);
		if (dialect == nullptr) {
			return false;
		}

		const Filetype *filetype = nullptr;
		for (std::string ext : dialect->filetypes) {
			filetype = getFiletype(ext);
			if (filetype != nullptr and filetype->write != nullptr) {
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
			filetype->write((emitDir / filename).string(), *this, *filetype, prgm, id);
		} else if (filetype->level == Filetype::MODULE) {
			fs::path emitDir = rootDir / BUILD / rootpathFromModule(prgm.mods[id.mod].name);
			std::filesystem::create_directories(emitDir.string());

			string filename = "module." + filetype->ext;
			filetype->write((emitDir / filename).string(), *this, *filetype, prgm, id);

		} else {
			fs::path emitDir = rootDir / BUILD / rootpathFromModule(prgm.mods[id.mod].name);
			std::filesystem::create_directories(emitDir.string());

			string filename = prgm.getPrototype(id).mangle(false) + "." + filetype->ext;
			filetype->write((emitDir / filename).string(), *this, *filetype, prgm, id);
		}
		return true;
	} else if (id.hasTerm()) {
		auto &term = prgm.termAt(id);
		if (term.variants.empty()) {
			return false;
		}

		std::vector<int> stack(1, 0);
		while (not stack.empty()) {
			id.var = stack.back();
			stack.pop_back();

			auto &var = term.variants[id.var];

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
				internal("", "dialect not defined for term '" + term.decl.name + "'", __FILE__, __LINE__);
				continue;
			}

			if (doExport) {
				save(prgm, id);
			}
		}
		return true;
	} else if (id.hasMod()) {
		bool result = true;
		for (id.index = 0; id.index < (int)prgm.modAt(id.mod).terms.size(); id.index++) {
			if (not save(prgm, id)) {
				result = false;
			}
		}
		return result;
	} else {
		bool result = true;
		for (id.mod = 0; id.mod < (int)prgm.mods.size(); id.mod++) {
			if (not prgm.modAt(id).isTech) {
				if (not save(prgm, id)) {
					result = false;
				}
			}
		}
		return result;
	}
	return false;
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
	return not rootDir.empty() and fs::exists(rootDir / "lm.mod");
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
	if (path.empty()) {
		path = workDir;
	} else if (not path.is_absolute()) {
		path = workDir / path;
	}

	fs::path dirInModule = fs::relative(path, rootDir).lexically_normal();
	std::string top = topDir(dirInModule);
	if (top == BUILD) {
		dirInModule = popTopDir(dirInModule);
		top = topDir(dirInModule);
	}
	
	if (top == ".") {
		return fs::path(modName).string();
	} else if (top == SOURCE) {
		dirInModule = popTopDir(dirInModule);
		if (dirInModule.empty()) {
			return fs::path(modName).string();
		}
		return (fs::path(modName) / dirInModule).string();
	} else if (top == VENDOR) {
		// TODO(edward.bingham) parse the lm.mod file in the vendor directory
		return popTopDir(dirInModule);
	}
	return dirInModule;
}

fs::path Project::pathFromModule(string mod) const {
	if (mod == modName) {
		return rootDir / SOURCE;
	} else if (mod.rfind(modName+"/", 0) == 0) {
		return rootDir / SOURCE / mod.substr(modName.size()+1);
	}
	return rootDir / VENDOR / mod;
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
