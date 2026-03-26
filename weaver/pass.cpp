#include "pass.h"

#include <filesystem>

#include <common/standard.h>
#include <common/timer.h>
#include <common/text.h>

namespace weaver {

Target::Target() {
	progress = false;
	debug = false;
}

Target::~Target() {
}

void Target::setEnum(std::string target, std::string value) {
	enums[target] = value;
}

std::string Target::getEnum(std::string target) const {
	auto i = enums.find(target);
	if (i == enums.end()) {
		return "";
	}
	return i->second;
}

void Target::setFlag(std::string target) {
	flags.insert(target);
}

bool Target::getFlag(std::string target) const {
	return (flags.find(target) != flags.end());
}

PassManager::PassManager() {
	proj = nullptr;
	target = nullptr;
	prgm = nullptr;
}

PassManager::~PassManager() {
}



RotationPlan PassManager::planRotations(TermId id, Predicate target) {
	/*struct Frame {
		TermId id;
		TermState state;
		RotationPlan plan;
		Predicate target;
	};

	std::vector<Frame> frames;
	frames.push_back({id, state[id.mod][id.index], RotationPlan(), target});
	while (not frames.empty()) {
		std::vector<Rotation> enabled;
		for (size_t i = 0; i < rotations.size(); i++) {
			for (size_t j =  

		if (curr < 0) {
			return RotationPlan();
		};
	}*/
}

bool PassManager::build() {
	if (proj == nullptr) {
		printf("error: select the project to build\n");
	}
	if (target == nullptr) {
		printf("error: specify the build target\n");
	}
	if (prgm == nullptr) {
		printf("error: select the program to build\n");
	}
	if (proj == nullptr or target == nullptr or prgm == nullptr) {
		return false;
	}

	/*int index = (int)todo.size()-1;
	while (index >= 0) {
		for (index = (int)todo.size()-1; index >= 0; index--) {
			TermId curr = todo[index];

			// TODO(edward.bingham) This just blindly applies passes
			// until it can't. Instead I should compute the possible
			// passes required to reach the end state.
			vector<array<int, 2> > opts;
			bool done = true;
			for (int i = 0; i < (int)passes.size(); i++) {
				if (not passes[i].enabled(*this)) {
					continue;
				}

				
				int ready = passes[i].ready(*this, prgm, curr);
				done = ready == -2 and done;
				if (ready >= 0) {
					opts.push_back({i, ready});
					// TODO(edward.bingham) we could check multiple
					// passes and then handle most optimal ordering here
					break;
				}
			}

			if (done) {
				todo.erase(todo.begin() + index);
			} else if (not opts.empty()) {
				passes[opts.back()[0]].run(*this, prgm, curr, opts.back()[1]);
				break;
			}
		}
	}
	return true;*/
}

}
