#pragma once

#include <map>
#include <vector>
#include <string>
#include <any>

namespace weaver {

// This is used to store data from analysis passes.
struct Metadata {
	enum {
		NONE = -2,       // Invalid term kind
		CONTEXT = -1,    // Context term (special term type for handling contexts)
		PROCESS = 0,     // Process term (regular function/process)
	};

	int kind;

	// This stores two different things:
	// 1. an analysis where i->first is the name and
	//    i->second is the result of the analysis
	// 2. a property guarantee where i->first is the name
	//    and i->second is the region or set of regions of
	//    the dialect for which that property is
	//    guaranteed or empty for whole process.
	std::map<std::string, std::any> props;
	double cost;

	Metadata(int kind=-1);
	~Metadata(); 

	std::string dialect() const;

	void set(std::string name);
	bool has(std::string name) const;
	void unset(std::string name);

	void print() const;

	template <typename T>
	const T *get(std::string name) const {
		auto i = props.find(name);
		if (i != props.end()) {
			return std::any_cast<T>(&i->second);
		}
		return nullptr;
	}

	template <typename T>
	T *get(std::string name) {
		auto i = props.find(name);
		if (i != props.end()) {
			return std::any_cast<T>(&i->second);
		}
		return nullptr;
	}

	template <typename T>
	T *set(std::string name, T value) {
		auto i = props.insert(pair<std::string, std::any>(name, value));
		return std::any_cast<T>(&i.first->second);
	}
};

}
