#include "compiler/type-reducing.h"
#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "compiler/ast.h"
#include "base/io.h"
#include "base/env.h"
#include "base/arena.h"
#include <gtest/gtest.h>

namespace yalx {

namespace cpl {

class TypeReducingTest : public ::testing::Test {
public:
    class MockErrorFeedback : public SyntaxFeedback {
    public:
        void DidFeedback(const SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(), location.begin_column(),
                     location.end_line(), location.end_column(), z);
        }
        
        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback
    
    TypeReducingTest() {}
    
    void SetUp() override {}
    void TearDown() override {}
    
    std::vector<std::string> MakeSearchPaths(const char *project_name) {
        std::vector<std::string> search_paths;
        std::string prefix("tests");
        prefix.append("/").append(project_name);
        std::string path(prefix);
        path.append("/src");
        search_paths.push_back(path);
        path.assign(prefix);
        path.append("/pkg");
        search_paths.push_back(path);
        search_paths.push_back("libs");
        return search_paths;
    }
protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    
}; // class TypeReducingTest


TEST_F(TypeReducingTest, Sanity) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/06-type-reducing-sanity", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    {
        std::unordered_map<std::string_view, GlobalSymbol> symbols;
        rs = ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
        ASSERT_TRUE(rs.ok()) << rs.ToString();

        auto iter = symbols.find("main:main.Foo");
        ASSERT_TRUE(iter != symbols.end());
        ASSERT_TRUE(iter->second.ast->IsClassDefinition());
        auto foo_class = iter->second.ast->AsClassDefinition();
        ASSERT_NE(nullptr, foo_class);
        ASSERT_NE(nullptr, foo_class->base_of());
        ASSERT_STREQ("Any", foo_class->base_of()->name()->data());
        
        auto fun = foo_class->method(0);
        ASSERT_EQ(1, fun->prototype()->return_types_size());
        ASSERT_EQ(Type::kType_i32, fun->prototype()->return_type(0)->primary_type());
    }
}

TEST_F(TypeReducingTest, ClassVars) {
    // 07-class-var-reducing
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/07-class-var-reducing", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    auto iter = symbols.find("main:main.Foo");
    ASSERT_TRUE(iter != symbols.end());
    auto foo_class = iter->second.ast->AsClassDefinition();
    ASSERT_NE(nullptr, foo_class);
    ASSERT_EQ(3, foo_class->fields_size());
    auto c_field = foo_class->field(2);
    EXPECT_STREQ("c", c_field.declaration->Identifier()->data());
    EXPECT_EQ(Type::kType_i32, c_field.declaration->Type()->primary_type());
    
}


TEST_F(TypeReducingTest, FileDeps) {
    // 07-class-var-reducing
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/08-file-unit-deps", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
}

TEST_F(TypeReducingTest, TmplDeps) {
    // 07-class-var-reducing
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/09-tmpl-inst-01", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(symbols.find("foo:foo.Foo<i32>") != symbols.end());
    ASSERT_TRUE(symbols.find("foo:foo.Baz<i32>") != symbols.end());
    
    auto clazz = symbols["foo:foo.Foo<i32>"].ast->AsClassDefinition();
    ASSERT_NE(nullptr, clazz);
    ASSERT_EQ(1, clazz->fields_size());
    ASSERT_EQ(Type::kType_i32, clazz->field(0).declaration->Type()->primary_type());
}

TEST_F(TypeReducingTest, IntefaceImpls) {
    // 07-class-var-reducing
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/10-interface-impls", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(symbols.find("yalx/lang:lang.I32") != symbols.end());
    ASSERT_TRUE(symbols.find("yalx/lang:lang.U32") != symbols.end());
}

} // namespace yalx

} // namespace yalx
