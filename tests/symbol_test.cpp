#include <gtest/gtest.h>
#include <weaver/symbol.h>

using namespace weaver;

// Helper function to set up a basic scope
Scope createTestScope() {
    Scope scope;
    TypeId typeId(1, 2);
    
    // Add some variables to the scope
    scope.tbl.push_back(Instance(typeId, "var1"));
    scope.tbl.push_back(Instance(typeId, "var2"));
    scope.tbl.push_back(Instance(typeId, "var3"));
    
    return scope;
}

// Test basic Scope functionality
TEST(ScopeTest, FindVariable) {
    Scope scope = createTestScope();
    
    // Test finding variables
    EXPECT_EQ(scope.find("var1"), 0);
    EXPECT_EQ(scope.find("var2"), 1);
    EXPECT_EQ(scope.find("var3"), 2);
    
    // Test finding a non-existent variable
    EXPECT_EQ(scope.find("nonexistent"), Scope::NOTFOUND);
}

// Test SymbolTable construction and basic scope management
TEST(SymbolTableTest, Construction) {
    SymbolTable symTable;
    
    // There should be at least one scope (the global scope)
    EXPECT_GT(symTable.scope.size(), 0u);
    
    // Create a new scope and test basic properties
    symTable.pushScope();
    EXPECT_EQ(symTable.scope.size(), 2u);
    
    // The new scope should have its parent set correctly
    EXPECT_EQ(symTable.scope.back().parent, 0);
    
    // The parent scope should have the new scope as a child
    EXPECT_EQ(symTable.scope[0].child.back(), 1);
}

// Test scope operations (push/pop)
TEST(SymbolTableTest, ScopeOperations) {
    SymbolTable symTable;
    int initialScope = symTable.curr;
    
    // Push a scope and verify current scope changes
    symTable.pushScope();
    EXPECT_EQ(symTable.curr, initialScope + 1);
    
    // Push another scope
    symTable.pushScope();
    EXPECT_EQ(symTable.curr, initialScope + 2);
    
    // Pop back to previous scope
    symTable.popScope();
    EXPECT_EQ(symTable.curr, initialScope + 1);
    
    // Pop back to initial scope
    symTable.popScope();
    EXPECT_EQ(symTable.curr, initialScope);
}

// Test variable definition and lookup
TEST(SymbolTableTest, VariableDefinition) {
    SymbolTable symTable;
    TypeId typeId(1, 2);
    
    // Define variables in the initial scope
    EXPECT_TRUE(symTable.define(Instance(typeId, "globalVar")));
    
    // Try to define a variable with the same name (should fail)
    EXPECT_FALSE(symTable.define(Instance(typeId, "globalVar")));
    
    // Create a new scope
    symTable.pushScope();
    
    // Define variables in the new scope
    EXPECT_TRUE(symTable.define(Instance(typeId, "localVar")));
    
    // Variable in global scope should be accessible
    Instance globalInstance = symTable.find("globalVar");
    EXPECT_EQ(globalInstance.name, "globalVar");
    
    // Variable in local scope should be accessible
    Instance localInstance = symTable.find("localVar");
    EXPECT_EQ(localInstance.name, "localVar");
    
    // Non-existent variable should return an empty instance
    Instance nonExistent = symTable.find("nonexistent");
    EXPECT_FALSE(nonExistent.type.defined());
    
    // Return to global scope
    symTable.popScope();
    
    // Local variable should no longer be accessible
    nonExistent = symTable.find("localVar");
    EXPECT_FALSE(nonExistent.type.defined());
}

// Test variable shadowing
TEST(SymbolTableTest, VariableShadowing) {
    SymbolTable symTable;
    TypeId typeId1(1, 2);
    TypeId typeId2(3, 4);
    
    // Define a variable in the global scope
    symTable.define(Instance(typeId1, "var"));
    
    // The variable should have typeId1
    Instance var = symTable.find("var");
    EXPECT_EQ(var.type.mod, 1);
    EXPECT_EQ(var.type.index, 2);
    
    // Create a new scope
    symTable.pushScope();
    
    // Define a variable with the same name but different type in the local scope
    symTable.define(Instance(typeId2, "var"));
    
    // The variable should now have typeId2 (shadowing the global one)
    var = symTable.find("var");
    EXPECT_EQ(var.type.mod, 3);
    EXPECT_EQ(var.type.index, 4);
    
    // Return to global scope
    symTable.popScope();
    
    // The variable should have typeId1 again
    var = symTable.find("var");
    EXPECT_EQ(var.type.mod, 1);
    EXPECT_EQ(var.type.index, 2);
} 
