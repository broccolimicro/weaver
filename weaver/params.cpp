#include "params.h"
#include <common/text.h>

namespace weaver {

std::map<std::string, std::string> readParams(const std::vector<std::string> &params) {
	std::map<std::string, std::string> result;
	for(const auto &line : params) {
		std::vector<std::string> args = splitArguments(line);
		for (const auto &arg : args) {
			size_t eq = arg.find_first_of("=");
			if (eq == std::string::npos) {
				continue;
			}
			std::string left = arg.substr(0, eq);
			std::string right = arg.substr(eq+1);
			if (right.size() >= 2u and right[0] == '"' and right.back() == '"') {
				right = right.substr(1, right.size()-1);
			}
			result.insert({left, right});
		}
	}
	return result;
}

}
