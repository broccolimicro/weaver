#include "proto.h"

#include <algorithm>
#include <common/hash.h>
#include <common/text.h>

namespace weaver {

const std::string label = "wv-";

Typename::Typename() {
}

Typename::Typename(std::string type) {
	parse(type);
}

Typename::~Typename() {
}

bool Typename::parse(std::string type) {
	size_t sqb = type.rfind("[");
	while (sqb != std::string::npos) {
		size.push_back(std::stoi(type.substr(sqb+1, type.size()-sqb-2)));
		type = type.substr(0, sqb);
		sqb = type.rfind("[");
	}
	std::reverse(size.begin(), size.end());

	size_t col = type.rfind(".");
	if (col != std::string::npos) {
		name = type.substr(col+1);
		mod = type.substr(0, col);
	} else {
		name = type;
	}
	return true;	
}

std::string Typename::to_string() const {
	std::string result = mod;
	if (not result.empty()) {
		result += ".";
	}
	result += name;
	for (auto i = size.begin(); i != size.end(); i++) {
		result += "[" + std::to_string(*i) + "]";
	}
	return result;
}

bool operator==(const Typename &t0, const Typename &t1) {
	if (t0.mod != t1.mod or t0.name != t1.name or t0.size.size() != t1.size.size()) {
		return false;
	}
	for (int i = 0; i < (int)t0.size.size(); i++) {
		if (t0.size[i] != t1.size[i]) {
			return false;
		}
	}
	return true;
}

size_t getHash(const std::vector<Typename> &args) {
	::hasher h;
	int count = args.size();
	h.put(&count);
	for (const Typename &arg : args) {
		h.put(arg.mod + "." + arg.name);
		for (int sz : arg.size) {
			h.put(&sz);
		}
	}
	return h.get();
}

Prototype::Prototype() {
	qualified = false;
	hashed = false;
	variant = -1;
	argsHash = 0;
}

Prototype::Prototype(std::string proto, std::string defaultMod) {
	variant = -1;
	argsHash = 0;
	mod = defaultMod;
	parse(proto);
}

Prototype::~Prototype() {
}

bool Prototype::parse(std::string proto) {
	qualified = false;
	hashed = false;

	size_t at = proto.rfind("@");
	if (at != std::string::npos) {
		std::string argStr = proto.substr(at+1, proto.size()-at-1);
		proto = proto.substr(0, at);
		variant = std::stoi(argStr);
	}

	size_t par = proto.rfind("(");
	if (par != std::string::npos) {
		qualified = true;
		std::string argStr = proto.substr(par+1, proto.size()-par-2);
		proto = proto.substr(0, par);

		size_t com = argStr.rfind(",");
		while (com != std::string::npos) {
			args.push_back(Typename());
			if (not args.back().parse(argStr.substr(com+1))) {
				return false;
			}
			argStr = argStr.substr(0, com);
			com = argStr.rfind(",");
		}
		if (not argStr.empty()) {
			args.push_back(Typename());
			if (not args.back().parse(argStr)) {
				return false;
			}
		}
	}
	if (qualified) {
		hashArgs();
	}

	size_t rec = proto.rfind("::");
	if (rec != std::string::npos) {
		name = proto.substr(rec+2);
		proto = proto.substr(0, rec);
	}

	size_t col = proto.rfind(".");
	if (col == std::string::npos) {
		name = proto;
		return true;
	}
	mod = proto.substr(0, col);
	if (name.empty()) {
		name = proto.substr(col+1);
	} else {
		recv = proto.substr(col+1);
	}
	return true;
}

std::string Prototype::to_string() const {
	std::string result = mod;
	if (not name.empty()) {
		if (not result.empty()) {
			result += ".";
		}
		if (qualified and not recv.empty()) {
			result += recv + "::";
		}

		result += name;
		if (qualified) {
			result += "(";
			for (int i = 0; i < (int)args.size(); i++) {
				if (i != 0) {
					result += ",";
				}
				result += args[i].to_string();
			}
			result += ")";
		}

		if (variant >= 0) {
			result += "@" + std::to_string(variant);
		}
	}
	return result;
}

bool Prototype::empty() const {
	return name.empty();
}

void Prototype::hashArgs() {
	argsHash = getHash(args);
	hashed = true;
}

std::string Prototype::mangle(bool useMod) const {
	std::string result;

	if (useMod) {
		result = mod;
		if (not result.empty()) {
			result += ".";
		}
	}

	if (not recv.empty()) {
		result += recv + "-";
	}
	result += name + "-" + encodeBase32(getHash(args));
	return label + result;
}

Prototype Prototype::fromMangled(std::string mangle, std::string defaultMod) {
	Prototype result;
	result.mod = defaultMod;
	if (mangle.rfind(label, 0) != 0) {
		result.name = mangle;
		return result;
	}

	mangle = mangle.substr(3);

	// drop the hashed argument list
	size_t pos = mangle.find_last_of('-');
	if (pos == string::npos) {
		result.name = mangle;
		return result;
	}

	result.argsHash = decodeBase32(mangle.substr(pos+1));
	result.hashed = true;
	mangle = mangle.substr(0, pos);

	// set the name
	pos = mangle.find_last_of("-.");
	if (pos == string::npos) {
		result.name = mangle;
		return result;
	}
	bool hasRecv = mangle[pos] == '-';

	result.name = mangle.substr(pos+1);
	mangle = mangle.substr(0, pos);

	// set the receiver
	if (hasRecv) {
		pos = mangle.find_last_of('.');
		if (pos == string::npos) {
			result.recv = mangle;
			return result;
		}

		result.recv = mangle.substr(pos+1);
		mangle = mangle.substr(0, pos);
	}

	// set the module
	result.mod = mangle;
	return result;
}

bool operator==(const Prototype &p0, const Prototype &p1) {
	if (not p0.mod.empty() and not p1.mod.empty() and p0.mod != p1.mod) {
		return false;
	}

	if (p0.recv != p1.recv) {
		return false;
	}

	if (p0.qualified and p1.qualified) {
		if (p0.args.size() != p1.args.size()) {
			return false;
		}
		for (int i = 0; i < (int)p0.args.size(); i++) {
			if (p0.args[i] != p1.args[i]) {
				return false;
			}
		}
		return true;
	}

	return p0.hashed and p1.hashed and p0.argsHash == p1.argsHash;
}

}
