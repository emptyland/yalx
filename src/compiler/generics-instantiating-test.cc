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
    
    void SetUp() override {}
    void TearDown() override {}
    
//    std::vector<std::string> MakeSearchPaths(const char *project_name) {
//        std::vector<std::string> search_paths;
//        std::string prefix("tests");
//        prefix.append("/").append(project_name);
//        std::string path(prefix);
//        path.append("/src");
//        search_paths.push_back(path);
//        path.assign(prefix);
//        path.append("/pkg");
//        search_paths.push_back(path);
//        search_paths.push_back("libs");
//        return search_paths;
//    }
    void SwitchInput(const char *z) {
        auto file = base::NewMemorySequentialFile(z, strlen(z));
        auto rs = parser_.SwitchInputFile(":memory:", file);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }
    
    
    class Resolver : public GenericsInstantiating::Resolver {
    public:
        Resolver() {}
        Statement *Find(std::string_view, std::string_view) override {
            return nullptr;
        }
        Statement *FindOrInsert(std::string_view, std::string_view, Statement *) override {
            return nullptr;
        }
    };
protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    Parser parser_;
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
    Type *argv[] = { new (&arena_) Type(&arena_, Type::kType_i32, {0,0}), };
    auto rs = GenericsInstantiating::Instantiate(file_unit->interface(0), &arena_, &feedback_, &resolver, 1, argv, &inst);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_TRUE(inst->IsInterfaceDefinition());
    auto if_def = inst->AsInterfaceDefinition();
    EXPECT_STREQ("main.Foo<i32>", if_def->full_name()->data());
    auto method = if_def->method(0);
    EXPECT_EQ(Type::kType_i32, method->prototype()->return_type(0)->primary_type());
}

} // namespace cpl

} // namespace yalx
