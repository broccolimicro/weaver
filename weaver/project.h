#pragma once

#include <parse/parse.h>
#include <weaver/program.h>
#include <weaver/proto.h>

#include <filesystem>

namespace fs = std::filesystem;

namespace weaver {

struct Project;
struct Filetype;

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
	typedef void (*Writer)(fs::path, Project &, const Filetype &, const Program &, int, int, int);

	string dialect;
	string ext;
	string build;

	std::any data;

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
	Filetype(string dialect, string ext, string build, Parser read, Loader load, Writer write, ConsolidationLevel level=TERM, std::any data = std::any());
	~Filetype();

	template <typename T>
	const T *as() const {
		return std::any_cast<T>(&data);
	}

	template <typename T>
	T *as() {
		return std::any_cast<T>(&data);
	}
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

struct Project {
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

	vector<Filetype> filetypes;

	Tech tech;

	int pushFiletype(string dialect, string ext, string build, Filetype::Parser read, Filetype::Loader load, Filetype::Writer write=nullptr, Filetype::ConsolidationLevel level=Filetype::TERM, std::any data=std::any());	
	const Filetype *getExtension(string ext) const;
	std::vector<const Filetype *> getDialect(string dialect) const;

	bool incl(std::string uri);
	bool read(Program &prgm, fs::path path);
	bool load(Program &prgm);

	bool save(Program &prgm, int modIdx, int termIdx, int varIdx);
	void save(Program &prgm, int modIdx, int termIdx);
	void save(Program &prgm);

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
