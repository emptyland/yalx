#include "compiler/generics-instantiating.h"
#include "compiler/compiler.h"
#include "compiler/parser.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "compiler/ast.h"
#include "base/io.h"
#include "base/env.h"
#include "base/arena.h"
#include <gtest/gtest.h>

namespace yalx {

namespace cpl {

class GenericsInstantiatingTest : public ::testing::Test {
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
    
    GenericsInstantiatingTest(): parser_(&arena_, &feedback_) {}
    
    void SetUp() override {
        i32_ = new (&arena_) Type(&arena_, Type::kType_i32, {0,0});
    }
    void TearDown() override {}

    void SwitchInput(const char *z) {
        auto file = base::NewMemorySequentialFile(z, strlen(z));
        auto rs = parser_.SwitchInputFile(":memory:", file);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }

    class Resolver : public GenericsInstantiating::Resolver {
    public:
        Resolver() {}
        Statement *Find(std::string_view prefix, std::string_view name) override {
            std::string buf(name.data(), name.size());
            if (!prefix.empty()) {
                buf.insert(0, ".");
                buf.insert(0, prefix);
            }
            if (auto iter = symbols.find(buf); iter == symbols.end()) {
                return nullptr;
            } else {
                return iter->second;
            }
        }
        Statement *FindOrInsert(std::string_view prefix, std::string_view name, Statement *ast) override {
            std::string buf(name.data(), name.size());
            if (!prefix.empty()) {
                buf.insert(0, ".");
                buf.insert(0, prefix);
            }
            if (auto iter = symbols.find(buf); iter == symbols.end()) {
                symbols[buf] = ast;
                return nullptr;
            } else {
                return iter->second;
            }
        }
        std::unordered_map<std::string, Statement *> symbols;
    };
    
    void Instantiate(Statement *ast, std::vector<Type *> argv, Statement **inst, GenericsInstantiating::Resolver *resolver) {
        auto rs = GenericsInstantiating::Instantiate(ast, &arena_, &feedback_, resolver, argv.size(), &argv[0], inst);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }
protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    Parser parser_;
    Type *i32_;
}; // class GenericsInstantiatingTest


TEST_F(GenericsInstantiatingTest, Sanity) {
    SwitchInput("package main\n"
                "\n"
                "interface Foo<T>{\n"
                "   fun foo(): T"
                "}\n"
                "\n");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, file_unit);
    
    Resolver resolver;
    Statement *inst = nullptr;
    Type *argv[] = { i32_, };
    auto rs = GenericsInstantiating::Instantiate(file_unit->interface(0), &arena_, &feedback_, &resolver, 1, argv, &inst);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    auto if_def = inst->AsInterfaceDefinition();
    EXPECT_STREQ("Foo<i32>", if_def->name()->data());
    auto method = if_def->method(0);
    EXPECT_EQ(Type::kType_i32, method->prototype()->return_type(0)->primary_type());
}

TEST_F(GenericsInstantiatingTest, SelfGenericsInstantiating) {
    SwitchInput("package main\n"
                "\n"
                "interface Foo<T>{\n"
                "   fun foo(): Foo<T>"
                "   fun f1(): Foo<i32>"
                "   fun f2(): Foo<i64>"
                "}\n"
                "\n");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, file_unit);
    
    auto stmt = file_unit->interface(0);
    Statement *inst = nullptr;
    Resolver resolver;
    resolver.symbols["Foo"] = stmt;
    Instantiate(stmt, {i32_}, &inst, &resolver);
    ASSERT_NE(nullptr, inst);
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    auto ifdef = inst->AsInterfaceDefinition();
    ASSERT_EQ(3, ifdef->methods_size());
    ASSERT_EQ(resolver.symbols["main.Foo<i32>"], ifdef->method(0)->prototype()->return_type(0)->AsClassType()->definition());
    ASSERT_EQ(resolver.symbols["main.Foo<i32>"], ifdef->method(1)->prototype()->return_type(0)->AsClassType()->definition());
    ASSERT_EQ(resolver.symbols["main.Foo<i64>"], ifdef->method(2)->prototype()->return_type(0)->AsClassType()->definition());
    
}

} // namespace cpl

} // namespace yalx
