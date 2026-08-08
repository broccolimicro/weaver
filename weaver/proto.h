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

bool operator==(const Typename &t0, const Typename &t1);

size_t getHash(const std::vector<Typename> &args);

struct Prototype {
	Prototype();
	Prototype(std::string proto, std::string defaultMod="");
	~Prototype();

	std::string mod;
	std::string name;

	std::string recv;
	std::vector<Typename> args;
	size_t argsHash;
	bool qualified;
	bool hashed;

	int variant;

	bool parse(std::string proto);
	std::string to_string() const;
	bool empty() const;

	void hashArgs();

	std::string mangle(bool useMod=true) const;
	static Prototype fromMangled(std::string mangle, std::string defaultMod="");
};

bool operator==(const Prototype &p0, const Prototype &p1);

}
