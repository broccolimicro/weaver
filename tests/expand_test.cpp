#include <gtest/gtest.h>
#include <common/text.h>
#include <weaver/expand.h>

using namespace weaver;

// Helper function to create a program with nested types for testing
Program createNestedTypesProgram() {
	Program prog;

	// Create a module
	int modIdx = prog.getModule("TestModule");
	Module &mod = prog.mods[modIdx];

	// Create a leaf type (no members)
	Type leafType = Type::typeOf("Leaf");
	int leafIdx = mod.createType(leafType);

	// Create a branch type with leaf members
	Type branchType = Type::typeOf("Branch");
	branchType.members.push_back(Instance(TypeId(modIdx, leafIdx), "leaf1"));
	branchType.members.push_back(Instance(TypeId(modIdx, leafIdx), "leaf2"));
	int branchIdx = mod.createType(branchType);

	// Create a root type with branch and leaf members
	Type rootType = Type::typeOf("Root");
	rootType.members.push_back(Instance(TypeId(modIdx, branchIdx), "branch1"));
	rootType.members.push_back(Instance(TypeId(modIdx, branchIdx), "branch2"));
	rootType.members.push_back(Instance(TypeId(modIdx, leafIdx), "leaf3"));
	mod.createType(rootType);

	return prog;
}

// Test basic variable expansion
TEST(ExpandTest, Leaves) {
	Program prog = createNestedTypesProgram();
	VariableExpander expander(prog);
		
	TypeId leafIdx = prog.findType("", "Leaf", 0);
	expander.registerType(leafIdx);

	// Create an instance of the Root type
	TypeId rootTypeId = prog.findType("", "Root", 0);
	Instance rootInst(rootTypeId, "rootVar");

	// Start the expansion from the root instance
	expander.start(rootInst);

	// Collect all the expanded variables
	std::vector<std::string> expandedNames;
	while (!expander.done()) {
		Instance inst = expander.next();
		expandedNames.push_back(inst.name);
	}
 
	EXPECT_EQ(expandedNames.size(), 5u);
	EXPECT_EQ(expandedNames[0], "rootVar.branch1.leaf1");
	EXPECT_EQ(expandedNames[1], "rootVar.branch1.leaf2");
	EXPECT_EQ(expandedNames[2], "rootVar.branch2.leaf1");
	EXPECT_EQ(expandedNames[3], "rootVar.branch2.leaf2");
	EXPECT_EQ(expandedNames[4], "rootVar.leaf3");
}

// Test basic variable expansion
TEST(ExpandTest, Branches) {
	Program prog = createNestedTypesProgram();
	VariableExpander expander(prog);
		
	TypeId leafIdx = prog.findType("", "Branch", 0);
	expander.registerType(leafIdx);

	// Create an instance of the Root type
	TypeId rootTypeId = prog.findType("", "Root", 0);
	Instance rootInst(rootTypeId, "rootVar");

	// Start the expansion from the root instance
	expander.start(rootInst);

	// Collect all the expanded variables
	std::vector<std::string> expandedNames;
	while (!expander.done()) {
		Instance inst = expander.next();
		expandedNames.push_back(inst.name);
	}
   
	EXPECT_EQ(expandedNames.size(), 2u);
	EXPECT_EQ(expandedNames[0], "rootVar.branch1");
	EXPECT_EQ(expandedNames[1], "rootVar.branch2");
}

// Test basic variable expansion
TEST(ExpandTest, Root) {
	Program prog = createNestedTypesProgram();
	VariableExpander expander(prog);
		
	TypeId leafIdx = prog.findType("", "Root", 0);
	expander.registerType(leafIdx);

	// Create an instance of the Root type
	TypeId rootTypeId = prog.findType("", "Root", 0);
	Instance rootInst(rootTypeId, "rootVar");

	// Start the expansion from the root instance
	expander.start(rootInst);

	// Collect all the expanded variables
	std::vector<std::string> expandedNames;
	while (!expander.done()) {
		Instance inst = expander.next();
		expandedNames.push_back(inst.name);
	}
   
	EXPECT_EQ(expandedNames.size(), 1u);
	EXPECT_EQ(expandedNames[0], "rootVar");
}

// Test basic variable expansion
TEST(ExpandTest, Invalid) {
	Program prog = createNestedTypesProgram();
	VariableExpander expander(prog);
		
	// Create an instance of the Root type
	TypeId rootTypeId = prog.findType("", "Root", 0);
	Instance rootInst(rootTypeId, "rootVar");

	// Start the expansion from the root instance
	expander.start(rootInst);

	// Collect all the expanded variables
	std::vector<std::string> expandedNames;
	while (!expander.done()) {
		Instance inst = expander.next();
		expandedNames.push_back(inst.name);
	}
   
	EXPECT_EQ(expandedNames.size(), 0u);
}
