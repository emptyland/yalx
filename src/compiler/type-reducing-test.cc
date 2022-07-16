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
        rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
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
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
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
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/08-file-unit-deps", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    auto deps = main_pkg->FindDepsOrNull("b");
    ASSERT_NE(nullptr, deps);
    ASSERT_TRUE(deps->ast()->IsVariableDeclaration());
    ASSERT_EQ(1, deps->backwards_size());
    ASSERT_EQ("a", deps->backward(0)->name());
    deps = deps->backward(0);
    ASSERT_EQ(0, deps->backwards_size());
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
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
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
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(symbols.find("yalx/lang:lang.I32") != symbols.end());
    ASSERT_TRUE(symbols.find("yalx/lang:lang.U32") != symbols.end());
}

TEST_F(TypeReducingTest, ObjectDecls) {
    // 07-class-var-reducing
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/11-object-decls", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(symbols.find("foo:foo.Foo$ShadowClass") != symbols.end());
    ASSERT_TRUE(symbols.find("foo:foo.Foo") != symbols.end());
    ASSERT_TRUE(symbols.find("main:main.baz") != symbols.end());
    
    auto fun = symbols["main:main.baz"].ast->AsFunctionDeclaration();
    ASSERT_NE(nullptr, fun);
    ASSERT_EQ(1, fun->prototype()->return_types_size());
    ASSERT_EQ(Type::kType_i32, fun->prototype()->return_type(0)->primary_type());
}

TEST_F(TypeReducingTest, Calling01) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/12-calling-01", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(symbols.find("main:main.gv") != symbols.end());
    
    auto val = down_cast<VariableDeclaration::Item>(symbols["main:main.gv"].ast);
    ASSERT_NE(nullptr, val);
    ASSERT_EQ(Type::kType_struct, val->Type()->primary_type());
}

// 13-while-loop-01
TEST_F(TypeReducingTest, WhileLoop01) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/13-while-loop-01", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(symbols.find("main:main.main") != symbols.end());
}

// 14-type-casting-01
TEST_F(TypeReducingTest, TypeCasting01) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/14-type-casting-01", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(symbols.find("main:main.v1") != symbols.end());
    auto v1 = down_cast<VariableDeclaration::Item>(symbols["main:main.v1"].ast);
    ASSERT_NE(nullptr, v1);
    ASSERT_EQ(Type::kType_i8, v1->type()->primary_type());
    
    auto v6 = down_cast<VariableDeclaration::Item>(symbols["main:main.v6"].ast);
    ASSERT_NE(nullptr, v6);
    ASSERT_EQ(Type::kType_u32, v6->type()->primary_type());
    
    auto v11 = down_cast<VariableDeclaration::Item>(symbols["main:main.v11"].ast);
    ASSERT_NE(nullptr, v11);
    ASSERT_TRUE(v11->type()->IsClassType());
    EXPECT_STREQ("C1", v11->type()->AsClassType()->definition()->name()->data());
    
    auto v12 = down_cast<VariableDeclaration::Item>(symbols["main:main.v12"].ast);
    ASSERT_NE(nullptr, v12);
    ASSERT_TRUE(v12->type()->IsClassType());
    EXPECT_STREQ("C3", v12->type()->AsClassType()->definition()->name()->data());

    auto v26 = down_cast<VariableDeclaration::Item>(symbols["main:main.v26"].ast);
    ASSERT_NE(nullptr, v26);
    ASSERT_TRUE(v26->type()->IsOptionType());
    EXPECT_EQ(Type::kType_i32, v26->type()->AsOptionType()->element_type()->primary_type());
}

// 15-when-expr-reducing
TEST_F(TypeReducingTest, WhenExprReducing) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/15-when-expr-reducing", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    auto v2 = down_cast<VariableDeclaration::Item>(symbols["main:main.v2"].ast);
    ASSERT_NE(nullptr, v2);
    ASSERT_EQ(Type::kType_i32, v2->type()->primary_type());
    
    auto v3 = down_cast<VariableDeclaration::Item>(symbols["main:main.v3"].ast);
    ASSERT_NE(nullptr, v3);
    EXPECT_TRUE(OptionType::DoesElementIs(v3->type(), Type::kType_i32));
    
    auto v4 = down_cast<VariableDeclaration::Item>(symbols["main:main.v4"].ast);
    ASSERT_NE(nullptr, v4);
    EXPECT_TRUE(OptionType::DoesElementIs(v4->type(), Type::kType_i32));
    
    auto v5 = down_cast<VariableDeclaration::Item>(symbols["main:main.v5"].ast);
    ASSERT_NE(nullptr, v5);
    EXPECT_TRUE(OptionType::DoesElementIs(v5->type(), Type::kType_i32));
    
    auto v6 = down_cast<VariableDeclaration::Item>(symbols["main:main.v6"].ast);
    ASSERT_NE(nullptr, v6);
    EXPECT_TRUE(OptionType::DoesElementIs(v6->type(), Type::kType_i32));
    
    auto v7 = down_cast<VariableDeclaration::Item>(symbols["main:main.v7"].ast);
    ASSERT_NE(nullptr, v7);
    EXPECT_TRUE(OptionType::DoesElementIs(v7->type(), Type::kType_i32));
}

// 16-try-catch-expr-reducing
TEST_F(TypeReducingTest, TryCatchExprReducing) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/16-try-catch-expr-reducing", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    auto v1 = down_cast<VariableDeclaration::Item>(symbols["main:main.v1"].ast);
    ASSERT_NE(nullptr, v1);
    ASSERT_EQ(Type::kType_i32, v1->type()->primary_type());
    
    auto v2 = down_cast<VariableDeclaration::Item>(symbols["main:main.v2"].ast);
    EXPECT_TRUE(OptionType::DoesElementIs(v2->type(), Type::kType_any));
    
    auto v3 = down_cast<VariableDeclaration::Item>(symbols["main:main.v3"].ast);
    EXPECT_TRUE(OptionType::DoesElementIs(v3->type(), Type::kType_i32));
}

// 17-simple-expr-reducing
TEST_F(TypeReducingTest, SimpleExprReducing) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/17-simple-expr-reducing", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
}

TEST_F(TypeReducingTest, ArrayExprReducing) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/26-ir-array-init-expr", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    auto a0 = down_cast<VariableDeclaration::Item>(symbols["main:main.a0"].ast);
    ASSERT_NE(nullptr, a0->type());
    ASSERT_TRUE(a0->type()->IsArrayType());
    auto ar = a0->type()->AsArrayType();
    ASSERT_EQ(1, ar->dimension_count());
    ASSERT_EQ(Type::kType_i32, ar->element_type()->primary_type());
    ASSERT_EQ(1, a0->owns()->initilaizers_size());
    ASSERT_EQ(a0->type(), down_cast<ArrayInitializer>(a0->owns()->initilaizer(0))->type());
    
    auto a1 = down_cast<VariableDeclaration::Item>(symbols["main:main.a1"].ast);
    ASSERT_NE(nullptr, a1->type());
    ASSERT_TRUE(a1->type()->IsArrayType());
    ar = a1->type()->AsArrayType();
    ASSERT_EQ(2, ar->dimension_count());
    ASSERT_EQ(Type::kType_i32, ar->GetActualElementType()->primary_type());
    ASSERT_EQ(1, a1->owns()->initilaizers_size());
    auto init = down_cast<ArrayInitializer>(a1->owns()->initilaizer(0));
    ASSERT_NE(nullptr, init);
    ASSERT_EQ(a1->type(), init->type());
    
    auto a2 = down_cast<VariableDeclaration::Item>(symbols["main:main.a2"].ast);
    ASSERT_NE(nullptr, a2->type());
    ASSERT_TRUE(a2->type()->IsArrayType());
    ar = a2->type()->AsArrayType();
    ASSERT_EQ(3, ar->dimension_count());
    ASSERT_EQ(Type::kType_i32, ar->GetActualElementType()->primary_type());
    init = down_cast<ArrayInitializer>(a2->owns()->initilaizer(0));
    ASSERT_NE(nullptr, init);
    ASSERT_EQ(a2->type(), init->type());
    
    auto a3 = down_cast<VariableDeclaration::Item>(symbols["main:main.a3"].ast);
    ASSERT_NE(nullptr, a3->type());
    ASSERT_TRUE(a3->type()->IsArrayType());
    ar = a3->type()->AsArrayType();
    ASSERT_EQ(1, ar->dimension_count());
    ASSERT_EQ(Type::kType_i32, ar->GetActualElementType()->primary_type());
    init = down_cast<ArrayInitializer>(a3->owns()->initilaizer(0));
    ASSERT_NE(nullptr, init);
    ASSERT_EQ(a3->type(), init->type());
    init = down_cast<ArrayInitializer>(init->dimension(0));
    ASSERT_NE(nullptr, init);
    ASSERT_EQ(1, init->dimension_count());
    
    
    auto a8 = down_cast<VariableDeclaration::Item>(symbols["main:main.a8"].ast);
    ASSERT_NE(nullptr, a8->type());
    ASSERT_TRUE(a8->type()->IsArrayType());
    ar = a8->type()->AsArrayType();
    ASSERT_EQ(2, ar->dimension_count());
    ASSERT_EQ(Type::kType_i32, ar->GetActualElementType()->primary_type());
    
    
    auto a9 = down_cast<VariableDeclaration::Item>(symbols["main:main.a9"].ast);
    ASSERT_NE(nullptr, a9->type());
    ASSERT_TRUE(a9->type()->IsArrayType());
    ar = a9->type()->AsArrayType();
    ASSERT_EQ(2, ar->dimension_count());
    ASSERT_EQ(3, ar->GetActualDimensionCount());
    ASSERT_TRUE(ar->element_type()->IsArrayType());
    ar = ar->element_type()->AsArrayType();
    ASSERT_EQ(1, ar->dimension_count());
}

TEST_F(TypeReducingTest, EnumTypeReducing) {
    base::ArenaMap<std::string_view, Package *> all(&arena_);
    base::ArenaVector<Package *> entries(&arena_);
    Package *main_pkg = nullptr;
    auto rs = Compiler::FindAndParseProjectSourceFiles("tests/29-ir-enum-types", "libs", &arena_, &feedback_,
                                                       &main_pkg, &entries, &all);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    std::unordered_map<std::string_view, GlobalSymbol> symbols;
    rs = Compiler::ReducePackageDependencesType(main_pkg, &arena_, &feedback_, &symbols);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    auto e0 = down_cast<VariableDeclaration::Item>(symbols["main:main.e0"].ast);
    ASSERT_NE(nullptr, e0->type());
    ASSERT_TRUE(e0->type()->IsEnumType());
    //auto ar = a0->type()->AsArrayType();
    auto em = e0->type()->AsEnumType();
    ASSERT_STREQ("foo:foo.Optional<i32>", em->definition()->FullName().c_str());
    
    auto e1 = down_cast<VariableDeclaration::Item>(symbols["main:main.e1"].ast);
    ASSERT_STREQ("foo:foo.Optional<i32>", e1->type()->AsEnumType()->definition()->FullName().c_str());
}

} // namespace yalx

} // namespace yalx
