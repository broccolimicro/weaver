#pragma once

#include <string>
#include <vector>

namespace weaver {

struct Typename {
	Typename();
	Typename(std::string type);
	~Typename();

	std::string mod;
	std::string name;
	std::vector<int> size;

	bool parse(std::string type);
	std::string to_string() const;
};

struct Prototype {
	Prototype();
	Prototype(std::string proto);
	~Prototype();

	std::string mod;
	std::string name;

	std::string recv;
	std::vector<Typename> args;
	size_t argsHash;
	bool unqualified;

	int variant;

	bool parse(std::string proto);
	std::string to_string() const;
	bool empty() const;

	size_t getHash() const;
	void hashArgs();

	std::string mangle(bool useMod=true) const;
	static Prototype fromMangled(std::string mangle);
};

}
