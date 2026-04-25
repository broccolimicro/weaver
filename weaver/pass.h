#pragma once

#include <common/standard.h>

#include <weaver/program.h>
#include <weaver/project.h>

#include <boolean/cover.h>

namespace weaver {

struct Target {
	std::map<std::string, std::string> enums;
	std::set<std::string> flags;

	bool progress;
	bool debug;

	Target();
	~Target();

	void setEnum(std::string target, std::string value);
	std::string getEnum(std::string target) const;
	void setFlag(std::string target);
	bool getFlag(std::string target) const;
};

struct Predicate {
	std::string dialect;
	boolean::cover expr;
};

struct PassManager;

// constrains design space, creates new variant
struct ReducePass {
	typedef bool (*Func)(const PassManager&, Program&, const Term &, const Variant&, Variant&);

	Predicate guard;
	boolean::cover depend;

	std::string dialect;
	boolean::cube action;
};

// no effect on design space, modifies variant
struct RotatePass {
	typedef bool (*Func)(const PassManager&, Program&, const Term&, Variant&);

	Predicate guard;
	boolean::cover depend;

	boolean::cube action;
};

struct State {
	std::string dialect;
	boolean::cube props;
};

typedef std::vector<State> TermState;
typedef std::vector<TermState> ModuleState;
typedef std::vector<ModuleState> ProgramState;

struct Rotation {
	TermId id;
	int variant;
	int pass;

	int prev;
};

typedef std::vector<Rotation> RotationPlan;

struct PassManager {
	const Project *proj;
	const Target *target;
	Program *prgm;
	std::vector<string> props;

	std::vector<ReducePass> reductions;
	std::vector<RotatePass> rotations;

	PassManager();
	~PassManager();

	RotationPlan planRotations(TermId id, Predicate target);
	bool build();
};

/*
enum {
		LOGIC_RAW = 0,
		LOGIC_CMOS = 1,
		LOGIC_ADIABATIC = 2,
	};

	enum {
		TIMING_MIXED = 0,
		TIMING_QDI = 1,
		TIMING_CLOCKED = 2,
	};

	enum {
		ELAB = 0,
		CONFLICTS = 1,
		ENCODE = 2,
		RULES = 3,
		BUBBLE = 4,
		KEEPERS = 5,
		SIZE = 6,
		NETS = 7,
		MAP = 8,
		CELLS = 9,
		PLACE = 10,
		ROUTE = 11
	};

	int logic;
	int timing;
	int stage;

	bool doPreprocess;
	bool doPostprocess;

	bool noCells;
	bool noGhosts;

	bool testDecompose;

	bool progress;
	bool debug;
	bool format_expressions_as_html_table;
*/

}
