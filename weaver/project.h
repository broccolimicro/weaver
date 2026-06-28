#pragma once

#include <parse/factory.h>
#include <weaver/program.h>
#include <weaver/proto.h>

#include <filesystem>

namespace fs = std::filesystem;

namespace weaver {

struct Project;
struct Filetype;
struct Dialect;

struct Depend {
	string path;
	string version;
};

struct Source {
	fs::path path;
	string modName;
	shared_ptr<parse::syntax> syntax;
	shared_ptr<tokenizer> tokens;
	const Filetype *filetype;
};

struct Filetype {
	// Project &proj, string path, string buffer, std::any data
	typedef void (*Parser)(Project &, Source &, string);
	// Project &proj, Program &prgm, string path, parse::syntax *syntax, std::any data
	typedef void (*Loader)(Project &, Program &, const Source &);
	// Program &prgm, int modIdx, int termIdx
	typedef void (*Writer)(fs::path, Project &, const Filetype &, const Program &, TermId id);

	string dialect;
	string ext;

	Parser read;
	Loader load;
	Writer write;

	enum ConsolidationLevel {
		PROJECT = 0,
		MODULE = 1,
		TERM = 2,
	};
	ConsolidationLevel level;

	Filetype();
	Filetype(string dialect, string ext, Parser read, Loader load, Writer write, ConsolidationLevel level=TERM);
	~Filetype();
};

struct Dialect {
	typedef std::any (*Load)(string name, const parse::syntax*, tokenizer*);
	typedef std::vector<TermId> (*Link)(const Project &, const Program &, const Variant &);

	string name;

	// filetypes (extension name) that directly support this dialect
	std::vector<std::string> filetypes;

	// DESIGN(edward.bingham) the default parser for a given dialect may not be
	// supported by a fully independent filetype, otherwise we'd just reference
	// that filetype instead of have parse() and load() in the dialect

	// parser
	const parse::factory *parse;

	// interpreter
	Load load;

	// compiler
	Link link;

	Dialect(std::string name, const parse::factory *parse, Load load, Link link);
	~Dialect();
};

struct Tech {
	std::string name;
	std::string path;
	std::vector<std::string> args;
	std::string lib;
	std::any def;

	template <typename T>
	const T *as() const {
		return std::any_cast<T>(&def);
	}

	template <typename T>
	T *as() {
		return std::any_cast<T>(&def);
	}
};

struct Project : parse::registry {
	Project(fs::path root="");
	~Project();

	static constexpr string DEBUG = "debug";
	static constexpr string BUILD = "build";
	static constexpr string VENDOR = "vendor";
	static constexpr string SOURCE = "src";

	vector<fs::path> includePath;

	fs::path workDir;
	fs::path rootDir;
	fs::path techDir;

	string modName;
	vector<Depend> depends;

	vector<fs::path> imports;
	vector<Source> sources;

	// indexed by extension
	std::map<std::string, Filetype> filetypes;

	// indexed by dialect name
	std::map<std::string, Dialect> dialects;

	Tech tech;

	bool pushDialect(string dialect, const parse::factory *parse, Dialect::Load load, Dialect::Link link);
	bool pushFiletype(string dialect, string ext, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write=nullptr, Filetype::ConsolidationLevel level=Filetype::TERM);
	const Filetype *getFiletype(string ext) const;
	Filetype *getFiletype(string ext);
	const Dialect *getDialect(string dialect) const;
	Dialect *getDialect(string dialect);

	const parse::factory *getParser(string dialect) const override;
	std::vector<std::string> getParserIndex() const override;

	bool incl(std::string uri);
	bool read(Program &prgm, fs::path path);
	bool load(Program &prgm);

	bool save(Program &prgm, TermId id=TermId());

	void setTech(string cmd);
	void setTechLib(string path);
	vector<string> listTech() const;

	bool hasMod() const;

	void vendor() const;
	void tidy();

	string pathToModule(fs::path path) const;
	fs::path pathFromModule(string mod) const;
	fs::path relpathFromModule(string mod) const;
	fs::path rootpathFromModule(string mod) const;

	fs::path buildPath(string dialect, string filename) const;
	fs::path debugPath(string dialect, string filename) const;
};

}
