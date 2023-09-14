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

#ifdef interface
#undef interface
#endif

namespace yalx::cpl {

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
        f32_ = new (&arena_) Type(&arena_, Type::kType_f32, {0,0});
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
        void Enter(Statement *stmt) override {
            
        }
        void Exit(Statement *stmt) override {
            
        }
        std::unordered_map<std::string, Statement *> symbols;
    };
    
    void Instantiate(Statement *ast, std::vector<Type *> argv, Statement **inst, GenericsInstantiating::Resolver *resolver) {
        auto rs = GenericsInstantiating::Instantiate(nullptr, ast, &arena_, &feedback_, resolver, argv.size(),
                                                     &argv[0], inst);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }
protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    Parser parser_;
    Type *i32_;
    Type *f32_;
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
    ASSERT_TRUE(nullptr != file_unit);
    
    Resolver resolver;
    Statement *inst = nullptr;
    Type *argv[] = { i32_, };
    auto rs = GenericsInstantiating::Instantiate(nullptr, file_unit->interface(0), &arena_, &feedback_, &resolver, 1,
                                                 argv, &inst);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    auto if_def = inst->AsInterfaceDefinition();
    EXPECT_STREQ("Foo<i32>", if_def->name()->data());
    auto method = if_def->method(0);
    EXPECT_EQ(Type::kType_i32, method->prototype()->return_type(0)->primary_type());
}

TEST_F(GenericsInstantiatingTest, SelfType) {
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
    ASSERT_TRUE(nullptr != file_unit);
    
    auto stmt = file_unit->interface(0);
    Statement *inst = nullptr;
    Resolver resolver;
    resolver.symbols["Foo"] = stmt;
    Instantiate(stmt, {i32_}, &inst, &resolver);
    ASSERT_TRUE(nullptr != inst);
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    auto ifdef = inst->AsInterfaceDefinition();
    ASSERT_EQ(3, ifdef->methods_size());
    ASSERT_EQ(resolver.symbols["main.Foo<i32>"], ifdef->method(0)->prototype()->return_type(0)->AsInterfaceType()->definition());
    ASSERT_EQ(resolver.symbols["main.Foo<i32>"], ifdef->method(1)->prototype()->return_type(0)->AsInterfaceType()->definition());
    ASSERT_EQ(resolver.symbols["main.Foo<i64>"], ifdef->method(2)->prototype()->return_type(0)->AsInterfaceType()->definition());
    
    Instantiate(stmt, {f32_}, &inst, &resolver);
    ASSERT_TRUE(nullptr != inst);
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    ifdef = inst->AsInterfaceDefinition();
    ASSERT_EQ(resolver.symbols["main.Foo<f32>"], ifdef->method(0)->prototype()->return_type(0)->AsInterfaceType()->definition());
    
}

TEST_F(GenericsInstantiatingTest, NestedType) {
    SwitchInput("package main\n"
                "\n"
                "interface Bar<T> {\n"
                "   fun bar(): T\n"
                "}\n"
                "interface Foo<T> {\n"
                "   fun foo(): T"
                "   fun f1(): Bar<f32>"
                "   fun f2(): Bar<f64>"
                "}\n"
                "\n");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(nullptr != file_unit);
    
    
    Resolver resolver;
    resolver.symbols["Bar"] = file_unit->interface(0);
    resolver.symbols["Foo"] = file_unit->interface(1);
    
    Statement *inst = nullptr;
    Instantiate(file_unit->interface(0), {i32_}, &inst, &resolver);
    ASSERT_TRUE(nullptr != inst);
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    
    auto bar_i32 = new (&arena_) InterfaceType(&arena_, inst->AsInterfaceDefinition(), inst->source_position());
    Instantiate(file_unit->interface(1), {bar_i32}, &inst, &resolver);
    ASSERT_TRUE(nullptr != inst);
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    
    auto foo_bar_i32 = inst->AsInterfaceDefinition();
    ASSERT_EQ(3, foo_bar_i32->methods_size());
    ASSERT_TRUE(foo_bar_i32->method(0)->prototype()->return_type(0)->IsInterfaceType());
    ASSERT_EQ(bar_i32->definition(), foo_bar_i32->method(0)->prototype()->return_type(0)->AsInterfaceType()->definition());
    
    ASSERT_TRUE(resolver.symbols.find("main.Bar<f32>") != resolver.symbols.end());
    ASSERT_TRUE(resolver.symbols.find("main.Bar<f64>") != resolver.symbols.end());
    
}

TEST_F(GenericsInstantiatingTest, RecursiveType) {
    SwitchInput("package main\n"
                "\n"
                "interface Foo<T>{\n"
                "   fun foo(): Foo<Foo<T> >"
                "}\n"
                "\n");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(nullptr != file_unit);

    auto stmt = file_unit->interface(0);
    Statement *inst = nullptr;
    Resolver resolver;
    resolver.symbols["Foo"] = stmt;
    Type *argv[] = {i32_};
    auto rs = GenericsInstantiating::Instantiate(nullptr, stmt, &arena_, &feedback_, &resolver, arraysize(argv), argv,
                                                 &inst);
    ASSERT_TRUE(rs.fail());
    ASSERT_EQ(nullptr, inst);
}

TEST_F(GenericsInstantiatingTest, NestedRecursiveType) {
    SwitchInput("package main\n"
                "\n"
                "interface Bar<T>{\n"
                "   fun bar(): Bar<T>\n"
                "}\n"
                "interface Foo<T>{\n"
                "   fun foo(): Foo<Bar<T> >\n"
                "}\n"
                "\n");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(nullptr != file_unit);
    
    Resolver resolver;
    resolver.symbols["Bar"] = file_unit->interface(0);
    resolver.symbols["Foo"] = file_unit->interface(1);
    
    Statement *inst = nullptr;
    Type *argv[] = {i32_};
    auto rs = GenericsInstantiating::Instantiate(nullptr, file_unit->interface(1), &arena_, &feedback_, &resolver,
                                                 arraysize(argv), argv, &inst);
    ASSERT_TRUE(rs.fail());
    ASSERT_EQ(nullptr, inst);
    
}

TEST_F(GenericsInstantiatingTest, EnumType) {
    SwitchInput(R"(package lang
    enum Optional<T> {
        None,
        Some(T)
    }
    )");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(nullptr != file_unit);
    
    Resolver resolver;
    Statement *inst = nullptr;
    Type *argv[] = {i32_};
    auto rs = GenericsInstantiating::Instantiate(nullptr, file_unit->enum_def(0), &arena_, &feedback_, &resolver,
                                                 arraysize(argv), argv, &inst);
    ASSERT_TRUE(rs.ok());
    ASSERT_TRUE(nullptr != inst);
    ASSERT_TRUE(inst->IsEnumDefinition());
    
    auto def = inst->AsEnumDefinition();
    ASSERT_STREQ("Optional<i32>", def->name()->data());
    ASSERT_EQ(2, def->fields_size());
    auto val = def->field(1).declaration;
    ASSERT_STREQ("Some", val->name()->data());
    ASSERT_EQ(Type::kType_i32, val->AtItem(0)->Type()->primary_type());
}

} // namespace yalx
