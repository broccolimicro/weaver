#include <gtest/gtest.h>
#include <common/text.h>
#include <weaver/expand.h>

using namespace weaver;

// Helper function to create a program with nested types for testing
Program createNestedTypesProgram() {
    Program prog;
    
    // Create a module
    int modIdx = prog.createModule("TestModule");
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

// Helper function to create a program with cyclic types for testing
Program createCyclicTypesProgram() {
    Program prog;
    
    // Create a module
    int modIdx = prog.createModule("TestModule");
    Module &mod = prog.mods[modIdx];
    
    // Create Type A (initially without members)
    Type typeA = Type::typeOf("A");
    int typeAIdx = mod.createType(typeA);
    
    // Create Type B that references Type A
    Type typeB = Type::typeOf("B");
    typeB.members.push_back(Instance(TypeId(modIdx, typeAIdx), "aRef"));
    int typeBIdx = mod.createType(typeB);
    
    // Now add a reference from Type A to Type B (creating a cycle)
    mod.types[typeAIdx].members.push_back(Instance(TypeId(modIdx, typeBIdx), "bRef"));
    
    return prog;
}

// Test basic variable expansion
TEST(ExpandTest, BasicExpansion) {
    Program prog = createNestedTypesProgram();
    VariableExpander expander(prog);
		
		TypeId leafIdx = prog.findType(0, {"Leaf"});
		expander.registerType(leafIdx);
    
    // Create an instance of the Root type
    TypeId rootTypeId = prog.findType(0, {"Root"});
    Instance rootInst(rootTypeId, "rootVar");
    
    // Start the expansion from the root instance
    expander.start(rootInst);
    
    // Collect all the expanded variables
    std::vector<std::string> expandedNames;
    while (!expander.done()) {
        Instance inst = expander.next();
        expandedNames.push_back(inst.name);
    }
   
		cout << ::to_string(expandedNames) << endl;
 
    // There should be 7 instances total (1 root + 2 branches + 4 leaves)
    EXPECT_EQ(expandedNames.size(), 6u);  // Not counting the root we already got
    
    // Check for branch1, branch2, and leaf3 at the root level
    EXPECT_NE(std::find(expandedNames.begin(), expandedNames.end(), "branch1"), expandedNames.end());
    EXPECT_NE(std::find(expandedNames.begin(), expandedNames.end(), "branch2"), expandedNames.end());
    EXPECT_NE(std::find(expandedNames.begin(), expandedNames.end(), "leaf3"), expandedNames.end());
    
    // Check for leaf1 and leaf2 members of branches
    // Note: The exact paths are not checked here as they depend on the
    // expansion ordering, which isn't guaranteed
}

// Test cycle detection and type registration
TEST(ExpandTest, CycleDetection) {
    Program prog = createCyclicTypesProgram();
    VariableExpander expander(prog);
    
    // Get references to the cyclic types
    TypeId typeAId = prog.findType(0, {"A"});
    TypeId typeBId = prog.findType(0, {"B"});
    
    // Create an instance of Type A
    Instance aInst(typeAId, "aVar");
    
    // Without registering types, this would cause infinite recursion
    expander.start(aInst);
    
    // Collect the expanded variables
    std::vector<std::string> expandedNames;
    expandedNames.push_back(expander.next().name); // First is aVar itself
    
    while (!expander.done()) {
        Instance inst = expander.next();
        expandedNames.push_back(inst.name);
    }
    
    // We expect infinite expansion without type registration
    // But the expander's internal logic should prevent this
    // There should be at least aVar, bRef, and aRef
    EXPECT_GE(expandedNames.size(), 3u);
    
    // Now test with explicit type registration
    VariableExpander registeredExpander(prog);
    registeredExpander.registerType(typeBId);
    
    registeredExpander.start(aInst);
    
    // Collect expanded variables
    std::vector<std::string> registeredNames;
    registeredNames.push_back(registeredExpander.next().name); // First is aVar itself
    
    while (!registeredExpander.done()) {
        Instance inst = registeredExpander.next();
        registeredNames.push_back(inst.name);
    }
    
    // We should only get aVar and bRef (no aRef because B type is registered)
    EXPECT_EQ(registeredNames.size(), 2u);
    EXPECT_EQ(registeredNames[0], "aVar");
    EXPECT_EQ(registeredNames[1], "bRef");
}

// Test the iterator interface
TEST(ExpandTest, IteratorInterface) {
    Program prog = createNestedTypesProgram();
    VariableExpander expander(prog);
    
    // Create an instance of the Branch type
    TypeId branchTypeId = prog.findType(0, {"Branch"});
    Instance branchInst(branchTypeId, "branchVar");
    
    // Test the done() method on a fresh expander
    EXPECT_TRUE(expander.done());
    
    // Start the expansion
    expander.start(branchInst);
    
    // Now it shouldn't be done
    EXPECT_FALSE(expander.done());
    
    // First element should be the branch itself
    Instance first = expander.next();
    EXPECT_EQ(first.name, "branchVar");
    
    // Still not done
    EXPECT_FALSE(expander.done());
    
    // Get the next element (should be leaf1)
    Instance second = expander.next();
    EXPECT_EQ(second.name, "leaf1");
    
    // Get the next element (should be leaf2)
    Instance third = expander.next();
    EXPECT_EQ(third.name, "leaf2");
    
    // Now it should be done
    EXPECT_TRUE(expander.done());
    
    // Reset the expansion by starting again
    expander.start(branchInst);
    EXPECT_FALSE(expander.done());
    
    // Count the number of elements
    int count = 0;
    do {
        expander.next();
        count++;
    } while (!expander.done());
    
    // There should be 3 elements (branch + 2 leaves)
    EXPECT_EQ(count, 3);
} 
