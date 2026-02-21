#include <gtest/gtest.h>
#include <weaver/program.h>

using namespace weaver;

// Setup helper function to create a test program with modules and types
Program createTestProgram() {
	Program prog;

	// Create modules
	int mod1 = prog.getModule("Mod1");
	int mod2 = prog.getModule("Mod2");

	// Add types to Mod1
	Type type1 = Type::typeOf("Type1");
	Type type2 = Type::typeOf("Type2");
	prog.mods[mod1].createType(type1);
	prog.mods[mod1].createType(type2);

	// Add types to Mod2
	Type type3 = Type::typeOf("Type3");
	prog.mods[mod2].createType(type3);

	// Load global types
	loadGlobalTypes(prog);

	return prog;
}

// Test type creation in modules
TEST(ProgramTest, TypeCreation) {
	Program prog;

	// Create a module
	int modIdx = prog.getModule("TestModule");

	// Create types
	Type regularType = Type::typeOf("RegularType");
	Type interfaceType = Type::interfaceOf("InterfaceType");

	// Add types to the module
	int typeIdx1 = prog.mods[modIdx].createType(regularType);
	int typeIdx2 = prog.mods[modIdx].createType(interfaceType);

	// Verify types were added correctly
	EXPECT_EQ(typeIdx1, 0);
	EXPECT_EQ(typeIdx2, 1);
	EXPECT_EQ(prog.mods[modIdx].types.size(), 2u);
	EXPECT_EQ(prog.mods[modIdx].types[typeIdx1].name, "RegularType");
	EXPECT_EQ(prog.mods[modIdx].types[typeIdx2].name, "InterfaceType");
	EXPECT_EQ(prog.mods[modIdx].types[typeIdx1].kind, Type::TYPE);
	EXPECT_EQ(prog.mods[modIdx].types[typeIdx2].kind, Type::INTERFACE);
}

// Test global types
TEST(ProgramTest, GlobalTypes) {
	Program prog;

	// Initially there should be no global module
	EXPECT_EQ(prog.global, -1);

	// Load global types
	loadGlobalTypes(prog);

	// Now there should be a global module
	EXPECT_GE(prog.global, 0);

	// Check the global module has the expected types
	Module &globalMod = prog.mods[prog.global];
	EXPECT_GT(globalMod.types.size(), 0u);

	// Find the expected built-in types
	bool foundChan = false;
	bool foundFixed = false;
	bool foundUfixed = false;
	bool foundBool = false;

	for (const Type &type : globalMod.types) {
		if (type.name == "chan") foundChan = true;
		if (type.name == "fixed") foundFixed = true;
		if (type.name == "ufixed") foundUfixed = true;
		if (type.name == "bool") foundBool = true;
	}

	EXPECT_TRUE(foundChan);
	EXPECT_TRUE(foundFixed);
	EXPECT_TRUE(foundUfixed);
	EXPECT_TRUE(foundBool);
}

// Test type lookup with unqualified names
TEST(ProgramTest, UnqualifiedTypeLookup) {
	Program prog = createTestProgram();

	// Look up types in Mod1
	TypeId type1Id = prog.findType("", "Type1", 0);
	TypeId type2Id = prog.findType("", "Type2", 0);

	EXPECT_TRUE(type1Id.defined());
	EXPECT_TRUE(type2Id.defined());
	EXPECT_EQ(type1Id.mod, 0);
	EXPECT_EQ(type1Id.index, 0);
	EXPECT_EQ(type2Id.mod, 0);
	EXPECT_EQ(type2Id.index, 1);

	// Look up a type that doesn't exist in Mod1
	TypeId nonExistentId = prog.findType("", "NonExistent", 0);
	EXPECT_FALSE(nonExistentId.defined());

	// Look up a global type from Mod1
	TypeId chanId = prog.findType("", "chan", 0);
	EXPECT_TRUE(chanId.defined());
	EXPECT_EQ(chanId.mod, prog.global);
}

// Test type lookup with qualified names
TEST(ProgramTest, QualifiedTypeLookup) {
	Program prog = createTestProgram();

	// Look up Mod1.Type1 from anywhere
	TypeId type1Id = prog.findType("Mod1", "Type1");
	EXPECT_TRUE(type1Id.defined());
	EXPECT_EQ(type1Id.mod, 0);

	// Look up Mod2.Type3 from anywhere
	TypeId type3Id = prog.findType("Mod2", "Type3");
	EXPECT_TRUE(type3Id.defined());
	EXPECT_EQ(type3Id.mod, 1);

	// Look up a non-existent qualified name
	TypeId nonExistentId = prog.findType("Mod1", "NonExistent");
	EXPECT_FALSE(nonExistentId.defined());

	// Look up a non-existent module
	TypeId badModuleId = prog.findType("NonExistentMod", "Type1");
	EXPECT_FALSE(badModuleId.defined());
}

// Test type access via TypeId
TEST(ProgramTest, TypeAccess) {
	Program prog = createTestProgram();

	// Look up a type
	TypeId typeId = prog.findType("", "Type1", 0);
	ASSERT_TRUE(typeId.defined());

	// Access the type 
	const Type &constType = prog.typeAt(typeId);
	EXPECT_EQ(constType.name, "Type1");

	// Modify the type
	Type &mutableType = prog.typeAt(typeId);
	mutableType.name = "ModifiedType";

	// Verify the modification
	EXPECT_EQ(prog.mods[typeId.mod].types[typeId.index].name, "ModifiedType");
} 
