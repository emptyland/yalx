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
    //modules["yalx/lang:lang"]->PrintTo(&printer);
    modules["foo:foo"]->PrintTo(&printer);
    printer.Write("\n");
    modules["main:main"]->PrintTo(&printer);
    
    static const char z[] = R"(module foo @foo:foo {
source-files:
    [0] tests/20-ir-gen-vtab/src/foo/foo.yalx

interfaces:
    interface If0 @foo:foo.If0 {
        fun doIt(): i32 ABSTRACT
        fun doThis(): i32 ABSTRACT
    } // foo:foo.If0

    interface If1 @foo:foo.If1 {
        fun doIt(): i32 ABSTRACT
        fun doThat(): i32 ABSTRACT
    } // foo:foo.If1

structures:
    class Foo @foo:foo.Foo {
        [base_of: @yalx/lang:lang.Any]
        vtab[8] = {
            Any::finalize
            Any::hashCode
            Any::id
            Any::isEmpty
            Foo::toString
            Foo::doIt
            Foo::doThis
            Foo::getName
        }
        itab[2] = {
        foo:foo.If0:
            Foo::doIt
            Foo::doThis
        }
    } // foo:foo.Foo

functions:
    fun $init(): void {
    boot:
        %0 = Closure ref[fun ()->void] <fun yalx/lang:lang.$init>
        CallRuntime void ref[fun ()->void] %0, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // foo:foo.$init

    fun Foo::toString(%this: ref[foo:foo.Foo]): string OVERRIDE {
    entry:
        Ret void string "foo"
    } // foo:foo.Foo.toString

    fun Foo::doThis(%this: ref[foo:foo.Foo]): i32 OVERRIDE {
    entry:
        Ret void i32 111
    } // foo:foo.Foo.doThis

    fun Foo::doIt(%this: ref[foo:foo.Foo]): i32 OVERRIDE {
    entry:
        Ret void i32 222
    } // foo:foo.Foo.doIt

    fun Foo::getName(%this: ref[foo:foo.Foo]): string {
    entry:
        Ret void string "name.foo"
    } // foo:foo.Foo.getName

    fun Foo::Foo$constructor(%this: ref[foo:foo.Foo]): ref[foo:foo.Foo] {
    entry:
        %0 = RefAssertedTo ref[yalx/lang:lang.Any] ref[foo:foo.Foo] %this
        %1 = CallHandle ref[yalx/lang:lang.Any] ref[yalx/lang:lang.Any] %0 <yalx/lang:lang.Any::Any$constructor>
        Ret void ref[foo:foo.Foo] %this
    } // foo:foo.Foo.Foo$constructor

} // @foo:foo

module main @main:main {
source-files:
    [0] tests/20-ir-gen-vtab/src/main/main.yalx

structures:
    class Bar @main:main.Bar {
        [base_of: @foo:foo.Foo]
        vtab[8] = {
            Any::finalize
            Any::hashCode
            Any::id
            Any::isEmpty
            Bar::toString
            Bar::doIt
            Bar::doThis
            Bar::getName
        }
        itab[4] = {
        foo:foo.If0:
            Bar::doIt
            Bar::doThis
        foo:foo.If1:
            Bar::doIt
            Bar::doThat
        }
    } // main:main.Bar

functions:
    fun $init(): void {
    boot:
        %0 = Closure ref[fun ()->void] <fun foo:foo.$init>
        CallRuntime void ref[fun ()->void] %0, string "foo:foo" <PkgInitOnce>
        %1 = Closure ref[fun ()->void] <fun yalx/lang:lang.$init>
        CallRuntime void ref[fun ()->void] %1, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // main:main.$init

    fun main(): void {
    entry:
        %0 = HeapAlloc ref[main:main.Bar] <Bar>
        %1 = CallHandle ref[main:main.Bar] ref[main:main.Bar] %0 <main:main.Bar::Bar$constructor>
        %2 = CallVirtual i32 ref[main:main.Bar] %1 <main:main.Bar::doIt>
        %3 = CallVirtual string ref[main:main.Bar] %1 <main:main.Bar::getName>
        %4 = RefAssertedTo ref[yalx/lang:lang.Any] ref[main:main.Bar] %1
        %5 = CallVirtual u32 ref[yalx/lang:lang.Any] %4 <yalx/lang:lang.Any::hashCode>
        Ret void
    } // main:main.main

    fun Bar::toString(%this: ref[main:main.Bar]): string OVERRIDE {
    entry:
        Ret void string "main"
    } // main:main.Bar.toString

    fun Bar::getName(%this: ref[main:main.Bar]): string OVERRIDE {
    entry:
        Ret void string "name.main"
    } // main:main.Bar.getName

    fun Bar::doIt(%this: ref[main:main.Bar]): i32 OVERRIDE {
    entry:
        Ret void i32 222
    } // main:main.Bar.doIt

    fun Bar::doThis(%this: ref[main:main.Bar]): i32 OVERRIDE {
    entry:
        Ret void i32 111
    } // main:main.Bar.doThis

    fun Bar::doThat(%this: ref[main:main.Bar]): i32 OVERRIDE {
    entry:
        Ret void i32 0
    } // main:main.Bar.doThat

    fun Bar::Bar$constructor(%this: ref[main:main.Bar]): ref[main:main.Bar] {
    entry:
        %0 = RefAssertedTo ref[foo:foo.Foo] ref[main:main.Bar] %this
        %1 = CallHandle ref[foo:foo.Foo] ref[foo:foo.Foo] %0 <foo:foo.Foo::Foo$constructor>
        Ret void ref[main:main.Bar] %this
    } // main:main.Bar.Bar$constructor

} // @main:main
)";
    
    ASSERT_EQ(z, buf);
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
source-files:
    [0] tests/21-ir-gen-while-loop/src/main/main.yalx

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

    fun issue7(): void {
    entry:
        Br void out [L1:]
    L1:
        %0 = Phi i32 i32 0, i32 %1 in [entry:, L2:]
        %2 = ICmp u8 i32 %0, i32 20 <slt>
        Br void u8 %2 out [L2:, L7:]
    L2:
        %1 = Add i32 i32 %0, i32 1
        %3 = ICmp u8 i32 %1, i32 5 <slt>
        Br void u8 %3 out [L3:, L4:]
    L3:
        Br void out [L1:]
        Br void out [L4:]
    L4:
        %4 = ICmp u8 i32 %1, i32 5 <sgt>
        Br void u8 %4 out [L5:, L6:]
    L5:
        Br void out [L7:]
        Br void out [L6:]
    L6:
        Br void out [L1:]
    L7:
        Ret void
    } // main:main.issue7

    fun issue8(): void {
    entry:
        Br void out [L6:]
    L1:
        %0 = Add i32 i32 %1, i32 1
        %2 = ICmp u8 i32 %0, i32 5 <slt>
        Br void u8 %2 out [L2:, L3:]
    L2:
        Br void out [L1:]
        Br void out [L3:]
    L3:
        %3 = ICmp u8 i32 %0, i32 5 <sgt>
        Br void u8 %3 out [L4:, L5:]
    L4:
        Br void out [L7:]
        Br void out [L5:]
    L5:
        Br void out [L6:]
    L6:
        %1 = Phi i32 i32 1, i32 %0 in [entry:, L1:]
        %4 = ICmp u8 i32 %1, i32 20 <slt>
        Br void u8 %4 out [L1:, L7:]
    L7:
        Ret void
    } // main:main.issue8

    fun main(): void {
    entry:
        Ret void
    } // main:main.main

} // @main:main
)";
    
    ASSERT_EQ(buf, z);
    //printf("%s\n", buf.c_str());
}

// 22-ir-gen-foreach-loop
TEST_F(IntermediateRepresentationGeneratorTest, ForeachLoop) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/22-ir-gen-foreach-loop", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["main:main"]->PrintTo(&printer);
    
    printf("%s\n", buf.c_str());
}

// 23-ir-gen-when-expr
TEST_F(IntermediateRepresentationGeneratorTest, WhenExpr) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arean_);
    IRGen("tests/23-ir-gen-when-expr", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["main:main"]->PrintTo(&printer);
    
    static const char z[] = R"(module main @main:main {
source-files:
    [0] tests/23-ir-gen-when-expr/src/main/main.yalx

functions:
    fun $init(): void {
    boot:
        %0 = Closure ref[fun ()->void] <fun foo:foo.$init>
        CallRuntime void ref[fun ()->void] %0, string "foo:foo" <PkgInitOnce>
        %1 = Closure ref[fun ()->void] <fun yalx/lang:lang.$init>
        CallRuntime void ref[fun ()->void] %1, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // main:main.$init

    fun main(): void {
    entry:
        Ret void
    } // main:main.main

    fun issue1(): void {
    entry:
        %0 = CallDirectly i32 <fun foo:foo.gen>
        Br void out [L1:]
    L1:
        %1 = ICmp u8 i32 %0, i32 1 <eq>
        Br void u8 %1 out [L2:, L5:]
    L2:
        %2 = ICmp u8 i32 %0, i32 2 <eq>
        Br void u8 %2 out [L3:, L5:]
    L3:
        %3 = ICmp u8 i32 %0, i32 3 <eq>
        Br void u8 %3 out [L4:, L5:]
    L4:
        CallDirectly void string "br1" <fun yalx/lang:lang.println>
        Br void out [L11:]
    L5:
        %4 = ICmp u8 i32 %0, i32 3 <slt>
        Br void u8 %4 out [L8:, L6:]
    L6:
        %5 = ICmp u8 i32 %0, i32 4 <sgt>
        Br void u8 %5 out [L8:, L7:]
    L7:
        CallDirectly void string "br2" <fun yalx/lang:lang.println>
        Br void out [L11:]
    L8:
        %6 = ICmp u8 i32 %0, i32 5 <sle>
        Br void u8 %6 out [L11:, L9:]
    L9:
        %7 = ICmp u8 i32 %0, i32 6 <sge>
        Br void u8 %7 out [L11:, L10:]
    L10:
        CallDirectly void string "br3" <fun yalx/lang:lang.println>
        Br void out [L11:]
    L11:
        Ret void
    } // main:main.issue1

    fun issue2(): void {
    entry:
        %0 = CallDirectly i32 <fun foo:foo.gen>
        Br void out [L1:]
    L1:
        %1 = CallDirectly i32 <fun foo:foo.foo>
        %2 = ReturningVal i32 i32 %1 <1>
        %3 = ReturningVal i32 i32 %1 <2>
        %4 = ICmp u8 i32 %0, i32 %1 <eq>
        Br void u8 %4 out [L2:, L5:]
    L2:
        %5 = ICmp u8 i32 %0, i32 %2 <eq>
        Br void u8 %5 out [L3:, L5:]
    L3:
        %6 = ICmp u8 i32 %0, i32 %3 <eq>
        Br void u8 %6 out [L4:, L5:]
    L4:
        CallDirectly void string "br1" <fun yalx/lang:lang.println>
        Br void out [L6:]
    L5:
        CallDirectly void string "else" <fun yalx/lang:lang.println>
        Br void out [L6:]
    L6:
        Ret void
    } // main:main.issue2

    fun issue3(): void {
    entry:
        %0 = CallDirectly i32 <fun foo:foo.gen>
        Br void out [L1:]
    L1:
        %1 = CallDirectly u8 <fun foo:foo.bar>
        Br void u8 %1 out [L2:, L3:]
    L2:
        Br void out [L4:]
    L3:
        Br void out [L4:]
    L4:
        %2 = Phi i32 i32 1, i32 2 in [L2:, L3:]
        %3 = ICmp u8 i32 %0, i32 %2 <eq>
        Br void u8 %3 out [L5:, L6:]
    L5:
        CallDirectly void string "br1" <fun yalx/lang:lang.println>
        Br void out [L6:]
    L6:
        Ret void
    } // main:main.issue3

    fun issue4(): void {
    entry:
        %0 = CallDirectly ref[yalx/lang:lang.Any] <fun foo:foo.baz>
        Br void out [L1:]
    L1:
        %1 = IsInstanceOf u8 ref[yalx/lang:lang.Any] %0 <Foo>
        Br void u8 %1 out [L2:, L3:]
    L2:
        %2 = RefAssertedTo ref[foo:foo.Foo] ref[yalx/lang:lang.Any] %0
        CallDirectly void string "br1" <fun yalx/lang:lang.println>
        Br void out [L5:]
    L3:
        %3 = IsInstanceOf u8 ref[yalx/lang:lang.Any] %0 <Foo>
        Br void u8 %3 out [L4:, L5:]
    L4:
        %4 = RefAssertedTo ref[foo:foo.Foo] ref[yalx/lang:lang.Any] %0
        %5 = LoadEffectField i32 ref[foo:foo.Foo] %4 <foo:foo.Foo::i>
        CallDirectly void string "br2" <fun yalx/lang:lang.println>
        Br void out [L5:]
    L5:
        Ret void
    } // main:main.issue4

    fun issue5(): i32 {
    entry:
        %0 = CallDirectly i32 <fun foo:foo.gen>
        Br void out [L1:]
    L1:
        %1 = ICmp u8 i32 %0, i32 1 <eq>
        Br void u8 %1 out [L2:, L5:]
    L2:
        %2 = ICmp u8 i32 %0, i32 2 <eq>
        Br void u8 %2 out [L3:, L5:]
    L3:
        %3 = ICmp u8 i32 %0, i32 3 <eq>
        Br void u8 %3 out [L4:, L5:]
    L4:
        Br void out [L11:]
    L5:
        %4 = ICmp u8 i32 %0, i32 3 <slt>
        Br void u8 %4 out [L8:, L6:]
    L6:
        %5 = ICmp u8 i32 %0, i32 4 <sgt>
        Br void u8 %5 out [L8:, L7:]
    L7:
        Br void out [L11:]
    L8:
        %6 = ICmp u8 i32 %0, i32 5 <sle>
        Br void u8 %6 out [L11:, L9:]
    L9:
        %7 = ICmp u8 i32 %0, i32 6 <sge>
        Br void u8 %7 out [L11:, L10:]
    L10:
        Br void out [L11:]
    L11:
        %8 = Phi i32 i32 0, i32 1, i32 2, i32 3 in [entry:, L4:, L7:, L10:]
        Ret void i32 %8
    } // main:main.issue5

    fun issue6(): ref[yalx/lang:lang.Any]? {
    entry:
        %0 = CallDirectly i32 <fun foo:foo.gen>
        Br void out [L1:]
    L1:
        %1 = ICmp u8 i32 %0, i32 1 <eq>
        Br void u8 %1 out [L2:, L5:]
    L2:
        %2 = ICmp u8 i32 %0, i32 2 <eq>
        Br void u8 %2 out [L3:, L5:]
    L3:
        %3 = ICmp u8 i32 %0, i32 3 <eq>
        Br void u8 %3 out [L4:, L5:]
    L4:
        Br void out [L12:]
    L5:
        %4 = ICmp u8 i32 %0, i32 3 <slt>
        Br void u8 %4 out [L8:, L6:]
    L6:
        %5 = ICmp u8 i32 %0, i32 4 <sgt>
        Br void u8 %5 out [L8:, L7:]
    L7:
        Br void out [L12:]
    L8:
        %6 = ICmp u8 i32 %0, i32 5 <sle>
        Br void u8 %6 out [L11:, L9:]
    L9:
        %7 = ICmp u8 i32 %0, i32 6 <sge>
        Br void u8 %7 out [L11:, L10:]
    L10:
        Br void out [L12:]
    L11:
        %8 = HeapAlloc ref[foo:foo.Foo] <Foo>
        %9 = CallHandle ref[foo:foo.Foo] ref[foo:foo.Foo] %8, i32 1 <foo:foo.Foo::Foo$constructor>
        Br void out [L12:]
    L12:
        %10 = RefAssertedTo ref[yalx/lang:lang.Any]? ref[foo:foo.Foo] %9
        %11 = Phi ref[yalx/lang:lang.Any]? string "br1", ref[yalx/lang:lang.Any]? nil, string "br2", ref[yalx/lang:lang.Any]? %10 in [L4:, L7:, L10:, L11:]
        Ret void ref[yalx/lang:lang.Any]? %11
    } // main:main.issue6

} // @main:main
)";
    //printf("%s\n", buf.c_str());
    ASSERT_EQ(z, buf);
}

} // namespace ir

} // namespace yalx
