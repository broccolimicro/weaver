#pragma once

#include <map>
#include <vector>
#include <string>
#include <any>

namespace weaver {

struct Condition {
	std::vector<std::vector<std::string> > analyses;
	std::string dialect;
};

// This is used to store data from analysis passes.
struct Metadata {
	enum {
		NONE = -2,       // Invalid term kind
		CONTEXT = -1,    // Context term (special term type for handling contexts)
		PROCESS = 0,     // Process term (regular function/process)
	};

	int kind;
	std::map<std::string, std::any> analysis;
	double cost;

	std::string dialect() const;

	void set(std::string name);
	bool has(std::string name) const;
	void unset(std::string name);
	bool meets(const Condition &cond) const;

	void print() const;

	template <typename T>
	T *findAnalysis(std::string name) {
		auto i = analysis.find(name);
		if (i != analysis.end()) {
			return std::any_cast<T>(&i->second);
		}
		return nullptr;
	}

	template <typename T>
	T *newAnalysis(std::string name, T value=T()) {
		auto i = analysis.insert(pair<std::string, std::any>(name, value));
		return std::any_cast<T>(&i.first->second);
	}
};

}
