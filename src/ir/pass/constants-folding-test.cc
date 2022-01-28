#include "ir/pass/constants-folding.h"
#include "ir/codegen.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/operators-factory.h"
#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class ConstantsFoldingPassTest : public ::testing::Test {
public:
    class MockErrorFeedback : public cpl::SyntaxFeedback {
    public:
        void DidFeedback(const cpl::SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(), location.begin_column(),
                     location.end_line(), location.end_column(), z);
        }
        
        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback
    
    ConstantsFoldingPassTest(): ops_(&arean_) {}
    
    void SetUp() override {}
    void TearDown() override {}
    
    void IRGen(const char *project_dir, base::ArenaMap<std::string_view, Module *> *modules, bool *ok) {
        base::ArenaMap<std::string_view, cpl::Package *> all(&ast_arean_);
        base::ArenaVector<cpl::Package *> entries(&ast_arean_);
        cpl::Package *main_pkg = nullptr;
        auto rs = cpl::Compiler::FindAndParseProjectSourceFiles(project_dir, "libs", &ast_arean_, &feedback_,
                                                                &main_pkg, &entries, &all);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
        std::unordered_map<std::string_view, cpl::GlobalSymbol> symbols;
        rs = cpl::Compiler::ReducePackageDependencesType(main_pkg, &ast_arean_, &feedback_, &symbols);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
        
        rs = cpl::Compiler::GenerateIntermediateRepresentationCode(symbols, &arean_, &ops_, main_pkg, &feedback_,
                                                                   modules);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
        *ok = true;
    }
    
    void RunPass(ConstantsFoldingPass::ModulesMap *modules) {
        ConstantsFoldingPass(&arean_, &ops_, modules, &feedback_).Run();
    }
    
protected:
    base::Arena ast_arean_;
    base::Arena arean_;
    OperatorsFactory ops_;
    MockErrorFeedback feedback_;
}; // class ConstantsFoldingPassTest

// 24-ir-constants-folding
TEST_F(ConstantsFoldingPassTest, Sanity) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/24-ir-constants-folding", &modules, &ok);
    ASSERT_TRUE(ok);
    RunPass(&modules);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["main:main"]->PrintTo(&printer);
    
    constexpr static const char z[] = R"(module main @main:main {
source-files:
    [0] tests/24-ir-constants-folding/src/main/main.yalx

globals:
    %0 = I32Constant i32 <100>
    %1 = I32Constant i32 <200>

functions:
    fun $init(): void {
    boot:
        %0 = Closure ref[fun ()->void] <fun yalx/lang:lang.$init>
        CallRuntime void ref[fun ()->void] %0, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // main:main.$init

    fun issue1(): i32 {
    entry:
        Ret void i32 300
    } // main:main.issue1

    fun issue2(): i32 {
    entry:
        Ret void i32 -100
    } // main:main.issue2

    fun issue3(): i32 {
    entry:
        Ret void i32 20000
    } // main:main.issue3

    fun issue4(): i32 {
    entry:
        %0 = SDiv i32 i32 100, i32 0
        Ret void i32 %0
    } // main:main.issue4

    fun issue5(): u8 {
    entry:
        Ret void u8 0
    } // main:main.issue5

    fun issue6(): u8, u8 {
    entry:
        Ret void u8 0, u8 1
    } // main:main.issue6

    fun main(): void {
    entry:
        Ret void
    } // main:main.main

} // @main:main
)";
    //printf("%s\n", buf.data());
    ASSERT_STREQ(z, buf.data());
}

} // namespace ir

} // namespace yalx
