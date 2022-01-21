#include "ir/codegen.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class IntermediateRepresentationGeneratorTest : public ::testing::Test {
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
    
    IntermediateRepresentationGeneratorTest() {}
    
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
        
        rs = cpl::Compiler::GenerateIntermediateRepresentationCode(symbols, &arean_, main_pkg, &feedback_, modules);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
        *ok = true;
    }

protected:
    base::Arena ast_arean_;
    base::Arena arean_;
    MockErrorFeedback feedback_;
}; // class IntermediateRepresentationGenerator


TEST_F(IntermediateRepresentationGeneratorTest, Sanity) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/18-ir-gen-sanity", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("foo:foo") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
//    modules["yalx/lang:lang"]->PrintTo(&printer);
//    modules["foo:foo"]->PrintTo(&printer);
    modules["main:main"]->PrintTo(&printer);
    
    //printf("%s\n", buf.data());
}

TEST_F(IntermediateRepresentationGeneratorTest, IfExpr) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/19-ir-gen-if-expr", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("foo:foo") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    //    modules["yalx/lang:lang"]->PrintTo(&printer);
    //    modules["foo:foo"]->PrintTo(&printer);
    modules["main:main"]->PrintTo(&printer);
    
    printf("%s\n", buf.data());
}

// 20-ir-gen-vtab
TEST_F(IntermediateRepresentationGeneratorTest, Vtab) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/20-ir-gen-vtab", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("foo:foo") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["yalx/lang:lang"]->PrintTo(&printer);
    modules["foo:foo"]->PrintTo(&printer);
    modules["main:main"]->PrintTo(&printer);
    
    printf("%s\n", buf.data());
}

// 21-ir-gen-while-loop
TEST_F(IntermediateRepresentationGeneratorTest, WhileLoop) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/21-ir-gen-while-loop", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("foo:foo") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["main:main"]->PrintTo(&printer);
    
    const char *z = R"(module main @main:main {
globals:
interfaces:
structures:
functions:
    fun $init(): void {
    boot:
        %0 = Closure ref[fun ()->void] <fun foo:foo.$init>
        CallRuntime void ref[fun ()->void] %0, string "foo:foo" <PkgInitOnce>
        %1 = Closure ref[fun ()->void] <fun yalx/lang:lang.$init>
        CallRuntime void ref[fun ()->void] %1, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // main:main.$init

    fun issue1(): void {
    entry:
        Br void out [L1:]
    L1:
        %0 = CallDirectly u8 <fun foo:foo.foo>
        Br void u8 %0 out [L2:, L3:]
    L2:
        CallDirectly void string "loop" <fun yalx/lang:lang.println>
        Br void out [L1:]
    L3:
        Ret void
    } // main:main.issue1

    fun issue2(): void {
    entry:
        Br void out [L2:]
    L1:
        CallDirectly void string "loop" <fun yalx/lang:lang.println>
        Br void out [L2:]
    L2:
        %0 = CallDirectly u8 <fun foo:foo.bar>
        Br void u8 %0 out [L1:, L3:]
    L3:
        Ret void
    } // main:main.issue2

    fun issue3(): void {
    entry:
        Br void out [L1:]
    L1:
        %0 = CallDirectly u8 <fun foo:foo.foo>
        Br void u8 %0 out [L3:, L2:]
    L2:
        CallDirectly void string "loop" <fun yalx/lang:lang.println>
        Br void out [L1:]
    L3:
        Ret void
    } // main:main.issue3

    fun issue4(): void {
    entry:
        Br void out [L2:]
    L1:
        CallDirectly void string "do {} unless" <fun yalx/lang:lang.println>
        Br void out [L2:]
    L2:
        %0 = CallDirectly u8 <fun foo:foo.bar>
        Br void u8 %0 out [L3:, L1:]
    L3:
        Ret void
    } // main:main.issue4

    fun issue5(): void {
    entry:
        Br void out [L1:]
    L1:
        %0 = Phi i32 i32 0, i32 %1 in [entry:, L5:]
        %2 = ICmp u8 i32 %0, i32 10 <slt>
        Br void u8 %2 out [L2:, L6:]
    L2:
        %3 = ICmp u8 i32 %0, i32 5 <slt>
        Br void u8 %3 out [L3:, L4:]
    L3:
        %4 = Add i32 i32 %0, i32 1
        Br void out [L5:]
    L4:
        %5 = Add i32 i32 %0, i32 2
        Br void out [L5:]
    L5:
        %1 = Phi i32 i32 %0, i32 %4, i32 %5 in [entry:, L3:, L4:]
        Br void out [L1:]
    L6:
        Ret void
    } // main:main.issue5

    fun issue6(): void {
    entry:
        Br void out [L2:]
    L1:
        %0 = Add i32 i32 %1, i32 1
        Br void out [L2:]
    L2:
        %1 = Phi i32 i32 0, i32 %0 in [entry:, L1:]
        %2 = ICmp u8 i32 %1, i32 20 <slt>
        Br void u8 %2 out [L1:, L3:]
    L3:
        Ret void
    } // main:main.issue6

    fun main(): void {
    entry:
        Ret void
    } // main:main.main

} // @main:main
)";
    
    ASSERT_EQ(buf, z);
    //printf("%s\n", z);
}

} // namespace ir

} // namespace yalx
