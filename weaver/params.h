#pragma once

#include <map>
#include <string>
#include <vector>

namespace weaver {

std::map<std::string, std::string> readParams(const std::vector<std::string> &params);

}
