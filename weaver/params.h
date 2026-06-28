#pragma once

#include <map>
#include <string>
#include <vector>

namespace weaver {

std::map<std::string, std::string> readParams(const std::vector<std::string> &params);
std::string writeParams(const std::map<std::string, std::string> &params);

}
