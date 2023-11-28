#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/base-test.h"
#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class IntermediateRepresentationGeneratorTest : public BaseTest {
public:
}; // class IntermediateRepresentationGenerator


TEST_F(IntermediateRepresentationGeneratorTest, Sanity) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
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
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
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
    static const char z[] = R"(module main @main:main {
source-files:
    [0] tests/19-ir-gen-if-expr/src/main/main.yalx

globals:
    %main:main.c = GlobalValue i32 <"main:main.c">
    %main:main.a = GlobalValue i32 <"main:main.a">
    %main:main.b = GlobalValue i32 <"main:main.b">
    %main:main.d = GlobalValue i32 <"main:main.d">

functions:
    fun $init(): void {
    boot:
        %0 = LoadFunAddr val[fun ()->void]* <fun foo:foo.$init>
        CallRuntime void val[fun ()->void]* %0, string "foo:foo" <PkgInitOnce>
        %1 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %1, string "yalx/lang:lang" <PkgInitOnce>
        StoreGlobal void i32 %main:main.d, i32 2
        %2 = LoadGlobal i32 i32 %main:main.d
        StoreGlobal void i32 %main:main.a, i32 1
        StoreGlobal void i32 %main:main.b, i32 %2
        %3 = CallDirectly u8 <fun foo:foo.foo>
        Br void u8 %3 out [L1:, L2:]
    L1:
        %4 = LoadGlobal i32 i32 %main:main.a
        Br void out [L9:]
    L2:
        %5 = CallDirectly u8 <fun foo:foo.bar>
        Br void u8 %5 out [L3:, L4:]
    L3:
        %6 = LoadGlobal i32 i32 %main:main.b
        Br void out [L8:]
    L4:
        %7 = CallDirectly u8 <fun foo:foo.baz>
        Br void u8 %7 out [L5:, L6:]
    L5:
        Br void out [L7:]
    L6:
        Br void out [L7:]
    L7:
        %8 = Phi i32 i32 3, i32 4 in [L5:, L6:]
        Br void out [L8:]
    L8:
        %9 = Phi i32 i32 %6, i32 %8 in [L3:, L7:]
        Br void out [L9:]
    L9:
        %10 = Phi i32 i32 %4, i32 %9 in [L1:, L8:]
        StoreGlobal void i32 %main:main.c, i32 %10
        Ret void
    } // main:main.$init

    fun issue1(): i32 {
    entry:
        %0 = CallDirectly u8 <fun foo:foo.foo>
        Br void u8 %0 out [L1:, L2:]
    L1:
        Br void out [L3:]
    L2:
        Br void out [L3:]
    L3:
        %1 = Phi i32 i32 1, i32 2 in [L1:, L2:]
        Ret void i32 %1
    } // main:main.issue1

    fun issue2(): val[yalx/lang:lang.Optional<foo:foo.Foo>] {
    entry:
        %0 = CallDirectly u8 <fun foo:foo.foo>
        Br void u8 %0 out [L1:, L2:]
    L1:
        %1 = HeapAlloc ref[foo:foo.Foo] <Foo>
        CallHandle void ref[foo:foo.Foo] %1, i32 1 <foo:foo.Foo::Foo$constructor>
        Br void out [L2:]
    L2:
        %2 = BitCastTo val[yalx/lang:lang.Optional<foo:foo.Foo>] ref[foo:foo.Foo] %1
        %3 = Phi val[yalx/lang:lang.Optional<foo:foo.Foo>] val[yalx/lang:lang.Optional<foo:foo.Foo>] %2, val[yalx/lang:lang.Optional<foo:foo.Foo>] nil in [L1:, entry:]
        Ret void val[yalx/lang:lang.Optional<foo:foo.Foo>] %3
    } // main:main.issue2

    fun issue3(): i32, i32 {
    entry:
        %0 = CallDirectly u8 <fun foo:foo.foo>
        Br void u8 %0 out [L1:, L2:]
    L1:
        %1 = VisitCallDirectly u8 <fun foo:foo.bar>
        Br void u8 %1 out [L3:, L4:]
    L2:
        Br void out [L6:]
    L3:
        Br void out [L5:]
    L4:
        Br void out [L5:]
    L5:
        %2 = Phi i32 i32 2, i32 3, i32 4 in [L1:, L3:, L4:]
        Br void out [L6:]
    L6:
        %3 = Phi i32 i32 1, i32 2, i32 %2, i32 3 in [entry:, L1:, L5:, L2:]
        %4 = Phi i32 i32 2, i32 1 in [entry:, L2:]
        Ret void i32 %3, i32 %4
    } // main:main.issue3

    fun main(): void {
    entry:
        Ret void
    } // main:main.main

} // @main:main
)";
    ASSERT_EQ(z, buf);
}

// 20-ir-gen-vtab
TEST_F(IntermediateRepresentationGeneratorTest, Vtab) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
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
        %0 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %0, string "yalx/lang:lang" <PkgInitOnce>
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

    fun Foo::Foo$constructor(%this: ref[foo:foo.Foo]): void {
    entry:
        %0 = RefAssertedTo ref[yalx/lang:lang.Any] ref[foo:foo.Foo] %this
        CallHandle void ref[yalx/lang:lang.Any] %0 <yalx/lang:lang.Any::Any$constructor>
        Ret void
        Ret void
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
        %0 = LoadFunAddr val[fun ()->void]* <fun foo:foo.$init>
        CallRuntime void val[fun ()->void]* %0, string "foo:foo" <PkgInitOnce>
        %1 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %1, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // main:main.$init

    fun main(): void {
    entry:
        %0 = HeapAlloc ref[main:main.Bar] <Bar>
        CallHandle void ref[main:main.Bar] %0 <main:main.Bar::Bar$constructor>
        %1 = CallVirtual i32 ref[main:main.Bar] %0 <main:main.Bar::doIt>
        %2 = CallVirtual string ref[main:main.Bar] %0 <main:main.Bar::getName>
        %3 = RefAssertedTo ref[yalx/lang:lang.Any] ref[main:main.Bar] %0
        %4 = CallVirtual u32 ref[yalx/lang:lang.Any] %3 <yalx/lang:lang.Any::hashCode>
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

    fun Bar::Bar$constructor(%this: ref[main:main.Bar]): void {
    entry:
        %0 = RefAssertedTo ref[foo:foo.Foo] ref[main:main.Bar] %this
        CallHandle void ref[foo:foo.Foo] %0 <foo:foo.Foo::Foo$constructor>
        Ret void
        Ret void
    } // main:main.Bar.Bar$constructor

} // @main:main
)";
    
    //printf("%s\n", buf.data());
    ASSERT_EQ(z, buf);
}

// 21-ir-gen-while-loop
TEST_F(IntermediateRepresentationGeneratorTest, WhileLoop) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
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
        %0 = LoadFunAddr val[fun ()->void]* <fun foo:foo.$init>
        CallRuntime void val[fun ()->void]* %0, string "foo:foo" <PkgInitOnce>
        %1 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %1, string "yalx/lang:lang" <PkgInitOnce>
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
        VisitCallDirectly void string "do {} unless" <fun yalx/lang:lang.println>
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
    
    //printf("%s\n", buf.c_str());
    ASSERT_EQ(buf, z);
    
}

// 22-ir-gen-foreach-loop
TEST_F(IntermediateRepresentationGeneratorTest, ForeachLoop) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
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
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
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
        %0 = LoadFunAddr val[fun ()->void]* <fun foo:foo.$init>
        CallRuntime void val[fun ()->void]* %0, string "foo:foo" <PkgInitOnce>
        %1 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %1, string "yalx/lang:lang" <PkgInitOnce>
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
        %1 = VisitCallDirectly u8 <fun foo:foo.bar>
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

    fun issue6(): val[yalx/lang:lang.Optional<any>] {
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
        CallHandle void ref[foo:foo.Foo] %8, i32 1 <foo:foo.Foo::Foo$constructor>
        Br void out [L12:]
    L12:
        %9 = BitCastTo val[yalx/lang:lang.Optional<any>] string "br1"
        %10 = BitCastTo val[yalx/lang:lang.Optional<any>] string "br2"
        %11 = BitCastTo val[yalx/lang:lang.Optional<any>] ref[foo:foo.Foo] %8
        %12 = Phi val[yalx/lang:lang.Optional<any>] val[yalx/lang:lang.Optional<any>] %9, val[yalx/lang:lang.Optional<any>] nil, val[yalx/lang:lang.Optional<any>] %10, val[yalx/lang:lang.Optional<any>] %11 in [L4:, L7:, L10:, L11:]
        Ret void val[yalx/lang:lang.Optional<any>] %12
    } // main:main.issue6

    fun issue7(): val[yalx/lang:lang.Optional<i32>] {
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
        Br void out [L8:]
    L5:
        %4 = ICmp u8 i32 %0, i32 4 <eq>
        Br void u8 %4 out [L6:, L7:]
    L6:
        Br void out [L8:]
    L7:
        Br void out [L8:]
    L8:
        %5 = StackAlloc val[yalx/lang:lang.Optional<i32>] <Optional<i32>>
        %6 = StoreInlineField val[yalx/lang:lang.Optional<i32>] val[yalx/lang:lang.Optional<i32>] %5, u16 1 <yalx/lang:lang.Optional<i32>::$enum_code$>
        %7 = StoreInlineField val[yalx/lang:lang.Optional<i32>] val[yalx/lang:lang.Optional<i32>] %6, i32 0 <yalx/lang:lang.Optional<i32>::Some>
        %8 = StackAlloc val[yalx/lang:lang.Optional<i32>] <Optional<i32>>
        %9 = StoreInlineField val[yalx/lang:lang.Optional<i32>] val[yalx/lang:lang.Optional<i32>] %8, u16 1 <yalx/lang:lang.Optional<i32>::$enum_code$>
        %10 = StoreInlineField val[yalx/lang:lang.Optional<i32>] val[yalx/lang:lang.Optional<i32>] %9, i32 1 <yalx/lang:lang.Optional<i32>::Some>
        %11 = StackAlloc val[yalx/lang:lang.Optional<i32>] <Optional<i32>>
        %12 = StoreInlineField val[yalx/lang:lang.Optional<i32>] val[yalx/lang:lang.Optional<i32>] %11, u16 0 <yalx/lang:lang.Optional<i32>::$enum_code$>
        %13 = Phi val[yalx/lang:lang.Optional<i32>] val[yalx/lang:lang.Optional<i32>] %7, val[yalx/lang:lang.Optional<i32>] %10, val[yalx/lang:lang.Optional<i32>] %12 in [L4:, L6:, L7:]
        Ret void val[yalx/lang:lang.Optional<i32>] %13
    } // main:main.issue7

} // @main:main
)";
    //printf("%s\n", buf.c_str());
    ASSERT_EQ(z, buf);
}

// 25-ir-throw-catch-expr
TEST_F(IntermediateRepresentationGeneratorTest, TryCatchExpression) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
    IRGen("tests/25-ir-throw-catch-expr", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("issue00:issue00") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["issue00:issue00"]->PrintTo(&printer);

    //printf("%s\n", buf.c_str());
}

TEST_F(IntermediateRepresentationGeneratorTest, ArrayInitExpression) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
    IRGen("tests/27-ir-array-init-expr2", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    //    modules["yalx/lang:lang"]->PrintTo(&printer);
    //    modules["foo:foo"]->PrintTo(&printer);
    modules["main:main"]->PrintTo(&printer);
    
    static const char z[] = R"(module main @main:main {
source-files:
    [0] tests/27-ir-array-init-expr2/src/main/main.yalx

functions:
    fun $init(): void {
    boot:
        %0 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %0, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // main:main.$init

    fun issue00(): ref[i32[,]] {
    entry:
        %0 = ArrayAlloc ref[i32[,]] i32 2, i32 2, i32 1, i32 2, i32 3, i32 4 <i32[,]>
        Ret void ref[i32[,]] %0
    } // main:main.issue00

    fun issue01(): ref[i32[]] {
    entry:
        %0 = ArrayAlloc ref[i32[]] i32 4, i32 1, i32 2, i32 3, i32 4 <i32[]>
        Ret void ref[i32[]] %0
    } // main:main.issue01

    fun issue02(): ref[i32[][]] {
    entry:
        %0 = ArrayAlloc ref[i32[]] i32 2, i32 1, i32 2 <i32[]>
        %1 = ArrayAlloc ref[i32[]] i32 2, i32 3, i32 4 <i32[]>
        %2 = ArrayAlloc ref[i32[]] i32 2, i32 5, i32 6 <i32[]>
        %3 = ArrayAlloc ref[i32[][]] i32 3, ref[i32[]] %0, ref[i32[]] %1, ref[i32[]] %2 <i32[][]>
        Ret void ref[i32[][]] %3
    } // main:main.issue02

    fun issue03(): ref[i32[,][]] {
    entry:
        %0 = ArrayAlloc ref[i32[]] i32 2, i32 1, i32 2 <i32[]>
        %1 = ArrayAlloc ref[i32[]] i32 2, i32 3, i32 4 <i32[]>
        %2 = ArrayAlloc ref[i32[]] i32 2, i32 5, i32 6 <i32[]>
        %3 = ArrayAlloc ref[i32[,][]] i32 3, i32 1, ref[i32[]] %0, ref[i32[]] %1, ref[i32[]] %2 <i32[,][]>
        Ret void ref[i32[,][]] %3
    } // main:main.issue03

    fun issue04(): ref[i32[][,]] {
    entry:
        %0 = ArrayAlloc ref[i32[,]] i32 1, i32 2, i32 1, i32 2 <i32[,]>
        %1 = ArrayAlloc ref[i32[,]] i32 1, i32 2, i32 3, i32 4 <i32[,]>
        %2 = ArrayAlloc ref[i32[,]] i32 1, i32 2, i32 5, i32 6 <i32[,]>
        %3 = ArrayAlloc ref[i32[][,]] i32 3, ref[i32[,]] %0, ref[i32[,]] %1, ref[i32[,]] %2 <i32[][,]>
        Ret void ref[i32[][,]] %3
    } // main:main.issue04

    fun issue05(): ref[i32[,,]] {
    entry:
        %0 = ArrayAlloc ref[i32[,,]] i32 3, i32 1, i32 2, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6 <i32[,,]>
        Ret void ref[i32[,,]] %0
    } // main:main.issue05

    fun issue06(): ref[i32[,][][,,]] {
    entry:
        %0 = ArrayAlloc ref[i32[,,]] i32 1, i32 1, i32 1, i32 1 <i32[,,]>
        %1 = ArrayAlloc ref[i32[][,,]] i32 1, ref[i32[,,]] %0 <i32[][,,]>
        %2 = ArrayAlloc ref[i32[,][][,,]] i32 1, i32 1, ref[i32[][,,]] %1 <i32[,][][,,]>
        Ret void ref[i32[,][][,,]] %2
    } // main:main.issue06

    fun issue07(): ref[i32[,]] {
    entry:
        %0 = ArrayFill ref[i32[,]] i32 4, i32 4, i32 0 <i32[,]>
        Ret void ref[i32[,]] %0
    } // main:main.issue07

    fun issue08(): ref[string[][]] {
    entry:
        %0 = ArrayFill ref[string[]] i32 4, string "hello" <string[]>
        %1 = ArrayFill ref[string[][]] i32 4, ref[string[]] %0 <string[][]>
        Ret void ref[string[][]] %1
    } // main:main.issue08

    fun issue09(): ref[i32[,][][,,]] {
    entry:
        %0 = ArrayFill ref[i32[,,]] i32 4, i32 4, i32 4, i32 1 <i32[,,]>
        %1 = ArrayFill ref[i32[][,,]] i32 3, ref[i32[,,]] %0 <i32[][,,]>
        %2 = ArrayFill ref[i32[,][][,,]] i32 2, i32 2, ref[i32[][,,]] %1 <i32[,][][,,]>
        Ret void ref[i32[,][][,,]] %2
    } // main:main.issue09

} // @main:main
)";
    //printf("%s\n", buf.c_str());
    ASSERT_EQ(z, buf);
}

TEST_F(IntermediateRepresentationGeneratorTest, PrimitiveProps) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
    IRGen("tests/28-ir-primitive-props", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    //    modules["yalx/lang:lang"]->PrintTo(&printer);
    //    modules["foo:foo"]->PrintTo(&printer);
    modules["main:main"]->PrintTo(&printer);
    //printf("%s\n", buf.c_str());
//    static const char z[] = R"(module main @main:main {
//source-files:
//    [0] tests/28-ir-primitive-props/src/main/main.yalx
//
//functions:
//    fun $init(): void {
//    boot:
//        %0 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
//        CallRuntime void val[fun ()->void]* %0, string "yalx/lang:lang" <PkgInitOnce>
//        Ret void
//    } // main:main.$init
//
//    fun issue00_size_of_array(): i32 {
//    entry:
//        %0 = ArrayAlloc ref[i32[]] i32 3, i32 1, i32 2, i32 3 <i32[]>
//        %1 = LoadEffectField i32 ref[i32[]] %0 <i32[]::size>
//        Ret void i32 %1
//    } // main:main.issue00_size_of_array
//
//    fun issue01_rank_of_array(): i32, i32 {
//    entry:
//        %0 = ArrayAlloc ref[i32[,]] i32 1, i32 1, i32 1 <i32[,]>
//        %1 = LoadEffectField i32 ref[i32[,]] %0 <i32[,]::rank>
//        %2 = LoadEffectField i32 ref[i32[,]] %0 <i32[,]::size>
//        Ret void i32 %1, i32 %2
//    } // main:main.issue01_rank_of_array
//
//    fun issue02_get_length_of_array(): i32 {
//    entry:
//        %0 = ArrayFill ref[i32[,,]] i32 2, i32 2, i32 2, i32 0 <i32[,,]>
//        %1 = CallHandle i32 ref[i32[,,]] %0, i32 0 <i32[,,]::getLength>
//        Ret void i32 %1
//    } // main:main.issue02_get_length_of_array
//
//} // @main:main
//)";
//    ASSERT_EQ(z, buf);
}

TEST_F(IntermediateRepresentationGeneratorTest, EnumValues) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
    IRGen("tests/29-ir-enum-types", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    //    modules["yalx/lang:lang"]->PrintTo(&printer);
    //    modules["foo:foo"]->PrintTo(&printer);
    modules["main:main"]->PrintTo(&printer);
    
    
    static const char z[] = R"(module main @main:main {
source-files:
    [0] tests/29-ir-enum-types/src/main/main.yalx

globals:
    %main:main.e0 = GlobalValue val[foo:foo.Optional<i32>] <"main:main.e0">
    %main:main.e1 = GlobalValue val[foo:foo.Optional<i32>] <"main:main.e1">
    %main:main.e2 = GlobalValue val[foo:foo.Foo] <"main:main.e2">
    %main:main.e3 = GlobalValue val[foo:foo.Foo] <"main:main.e3">
    %main:main.e4 = GlobalValue val[foo:foo.Foo] <"main:main.e4">
    %main:main.e5 = GlobalValue val[foo:foo.Foo] <"main:main.e5">

functions:
    fun $init(): void {
    boot:
        %0 = LoadFunAddr val[fun ()->void]* <fun foo:foo.$init>
        CallRuntime void val[fun ()->void]* %0, string "foo:foo" <PkgInitOnce>
        %1 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %1, string "yalx/lang:lang" <PkgInitOnce>
        %2 = StackAlloc val[foo:foo.Optional<i32>] <Optional<i32>>
        %3 = StoreInlineField val[foo:foo.Optional<i32>] val[foo:foo.Optional<i32>] %2, u16 0 <foo:foo.Optional<i32>::$enum_code$>
        StoreGlobal void val[foo:foo.Optional<i32>] %main:main.e0, val[foo:foo.Optional<i32>] %3
        %4 = StackAlloc val[foo:foo.Optional<i32>] <Optional<i32>>
        %5 = StoreInlineField val[foo:foo.Optional<i32>] val[foo:foo.Optional<i32>] %4, u16 1 <foo:foo.Optional<i32>::$enum_code$>
        %6 = StoreInlineField val[foo:foo.Optional<i32>] val[foo:foo.Optional<i32>] %5, i32 1 <foo:foo.Optional<i32>::Some>
        StoreGlobal void val[foo:foo.Optional<i32>] %main:main.e1, val[foo:foo.Optional<i32>] %6
        %7 = StackAlloc val[foo:foo.Foo] <Foo>
        %8 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %7, u16 0 <foo:foo.Foo::$enum_code$>
        StoreGlobal void val[foo:foo.Foo] %main:main.e2, val[foo:foo.Foo] %8
        %9 = StackAlloc val[foo:foo.Foo] <Foo>
        %10 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %9, u16 1 <foo:foo.Foo::$enum_code$>
        %11 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %10, string "ok" <foo:foo.Foo::B>
        StoreGlobal void val[foo:foo.Foo] %main:main.e3, val[foo:foo.Foo] %11
        %12 = StackAlloc val[foo:foo.Foo] <Foo>
        %13 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %12, u16 2 <foo:foo.Foo::$enum_code$>
        %14 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %13, u8 1 <foo:foo.Foo::C>
        StoreGlobal void val[foo:foo.Foo] %main:main.e4, val[foo:foo.Foo] %14
        %15 = StackAlloc val[foo:foo.Optional<i32>] <Optional<i32>>
        %16 = StoreInlineField val[foo:foo.Optional<i32>] val[foo:foo.Optional<i32>] %15, u16 1 <foo:foo.Optional<i32>::$enum_code$>
        %17 = StoreInlineField val[foo:foo.Optional<i32>] val[foo:foo.Optional<i32>] %16, i32 1 <foo:foo.Optional<i32>::Some>
        %18 = StackAlloc val[foo:foo.Foo] <Foo>
        %19 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %18, u16 3 <foo:foo.Foo::$enum_code$>
        %20 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %19, val[foo:foo.Optional<i32>] %17 <foo:foo.Foo::D>
        StoreGlobal void val[foo:foo.Foo] %main:main.e5, val[foo:foo.Foo] %20
        Ret void
    } // main:main.$init

    fun issue00_enum_value(): val[foo:foo.Foo] {
    entry:
        %0 = StackAlloc val[foo:foo.Foo] <Foo>
        %1 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %0, u16 0 <foo:foo.Foo::$enum_code$>
        Ret void val[foo:foo.Foo] %1
    } // main:main.issue00_enum_value

    fun issue01_enum_string_value(): val[foo:foo.Foo] {
    entry:
        %0 = StackAlloc val[foo:foo.Foo] <Foo>
        %1 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %0, u16 1 <foo:foo.Foo::$enum_code$>
        %2 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %1, string "hello" <foo:foo.Foo::B>
        Ret void val[foo:foo.Foo] %2
    } // main:main.issue01_enum_string_value

    fun issue02_enum_of_enum(): val[foo:foo.Foo] {
    entry:
        %0 = StackAlloc val[foo:foo.Optional<i32>] <Optional<i32>>
        %1 = StoreInlineField val[foo:foo.Optional<i32>] val[foo:foo.Optional<i32>] %0, u16 1 <foo:foo.Optional<i32>::$enum_code$>
        %2 = StoreInlineField val[foo:foo.Optional<i32>] val[foo:foo.Optional<i32>] %1, i32 997 <foo:foo.Optional<i32>::Some>
        %3 = StackAlloc val[foo:foo.Foo] <Foo>
        %4 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %3, u16 3 <foo:foo.Foo::$enum_code$>
        %5 = StoreInlineField val[foo:foo.Foo] val[foo:foo.Foo] %4, val[foo:foo.Optional<i32>] %2 <foo:foo.Foo::D>
        Ret void val[foo:foo.Foo] %5
    } // main:main.issue02_enum_of_enum

    fun issue03_enum_compact_value(): val[foo:foo.Optional<string>] {
    entry:
        %0 = BitCastTo val[foo:foo.Optional<string>] string "ok"
        Ret void val[foo:foo.Optional<string>] %0
    } // main:main.issue03_enum_compact_value

    fun issue04_compact_enum_matching(%e: val[foo:foo.Optional<string>]): void {
    entry:
        Br void out [L1:]
    L1:
        %0 = ICmp u8 val[foo:foo.Optional<string>] %e, val[foo:foo.Optional<string>] nil <eq>
        Br void u8 %0 out [L2:, L3:]
    L2:
        Br void out [L5:]
    L3:
        %1 = ICmp u8 val[foo:foo.Optional<string>] %e, val[foo:foo.Optional<string>] nil <eq>
        Br void u8 %1 out [L5:, L4:]
    L4:
        %2 = BitCastTo string val[foo:foo.Optional<string>] %e
        CallDirectly void string %2 <fun yalx/lang:lang.println>
        Br void out [L5:]
    L5:
        Ret void
    } // main:main.issue04_compact_enum_matching

    fun issue05_enum_matching(%e: val[foo:foo.Foo]): void {
    entry:
        Br void out [L1:]
    L1:
        %0 = LoadInlineField u16 val[foo:foo.Foo] %e <foo:foo.Foo::$enum_code$>
        %1 = ICmp u8 u16 %0, u16 0 <eq>
        Br void u8 %1 out [L2:, L3:]
    L2:
        Br void out [L9:]
    L3:
        %2 = LoadInlineField u16 val[foo:foo.Foo] %e <foo:foo.Foo::$enum_code$>
        %3 = ICmp u8 u16 %2, u16 1 <eq>
        Br void u8 %3 out [L4:, L5:]
    L4:
        %4 = LoadInlineField string val[foo:foo.Foo] %e <foo:foo.Foo::B>
        VisitCallDirectly void string %4 <fun yalx/lang:lang.println>
        Br void out [L9:]
    L5:
        %5 = LoadInlineField u16 val[foo:foo.Foo] %e <foo:foo.Foo::$enum_code$>
        %6 = ICmp u8 u16 %5, u16 2 <eq>
        Br void u8 %6 out [L6:, L7:]
    L6:
        %7 = LoadInlineField u8 val[foo:foo.Foo] %e <foo:foo.Foo::C>
        Br void out [L9:]
    L7:
        %8 = LoadInlineField u16 val[foo:foo.Foo] %e <foo:foo.Foo::$enum_code$>
        %9 = ICmp u8 u16 %8, u16 3 <eq>
        Br void u8 %9 out [L8:, L9:]
    L8:
        %10 = LoadInlineField val[foo:foo.Optional<i32>] val[foo:foo.Foo] %e <foo:foo.Foo::D>
        Br void out [L9:]
    L9:
        Ret void
    } // main:main.issue05_enum_matching

} // @main:main
)";
    //printf("%s\n", buf.c_str());
    ASSERT_EQ(z, buf);
}

TEST_F(IntermediateRepresentationGeneratorTest, ClosureCreating) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
    IRGen("tests/30-ir-gen-closure", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["main:main"]->PrintTo(&printer);
    
    static const char z[] = R"(module main @main:main {
source-files:
    [0] tests/30-ir-gen-closure/src/main/main.yalx

structures:
    class anonymous.fun0.closure @main:main.anonymous.fun0.closure {
        [base_of: @yalx/lang:lang.Any]
        $fun_entry$: qword
    } // main:main.anonymous.fun0.closure

    class anonymous.fun1.closure @main:main.anonymous.fun1.closure {
        [base_of: @yalx/lang:lang.Any]
        $fun_entry$: qword
        a: i32
    } // main:main.anonymous.fun1.closure

    class anonymous.fun2.closure @main:main.anonymous.fun2.closure {
        [base_of: @yalx/lang:lang.Any]
        $fun_entry$: qword
        a: i32
    } // main:main.anonymous.fun2.closure

functions:
    fun $init(): void {
    boot:
        %0 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %0, string "yalx/lang:lang" <PkgInitOnce>
        Ret void
    } // main:main.$init

    fun issue00_return_closure(): ref[fun (i32,i32)->(i32)] {
    entry:
        %0 = Closure ref[fun (main:main.anonymous.fun0.closure,i32,i32)->(i32)] <anonymous.fun0.closure>
        Ret void ref[fun (main:main.anonymous.fun0.closure,i32,i32)->(i32)] %0
    } // main:main.issue00_return_closure

    fun issue01_closure_capture(%a: i32): ref[fun (i32)->(i32)] {
    entry:
        %0 = Closure ref[fun (main:main.anonymous.fun1.closure,i32)->(i32)] i32 %a <anonymous.fun1.closure>
        Ret void ref[fun (main:main.anonymous.fun1.closure,i32)->(i32)] %0
    } // main:main.issue01_closure_capture

    fun issue02_closure_capture(): ref[fun (i32)->(i32)] {
    entry:
        %0 = Closure ref[fun (main:main.anonymous.fun2.closure,i32)->(i32)] i32 1 <anonymous.fun2.closure>
        Ret void ref[fun (main:main.anonymous.fun2.closure,i32)->(i32)] %0
    } // main:main.issue02_closure_capture

    fun anonymous.fun0.closure::apply(%callee: ref[main:main.anonymous.fun0.closure], %a: i32, %b: i32): i32 {
    entry:
        %0 = Add i32 i32 %a, i32 %b
        Ret void i32 %0
    } // anonymous.fun0.closure.apply

    fun anonymous.fun1.closure::apply(%callee: ref[main:main.anonymous.fun1.closure], %b: i32): i32 {
    entry:
        %0 = LoadEffectField i32 ref[main:main.anonymous.fun1.closure] %callee <main:main.anonymous.fun1.closure::a>
        %1 = Add i32 i32 %0, i32 %b
        Ret void i32 %1
    } // anonymous.fun1.closure.apply

    fun anonymous.fun2.closure::apply(%callee: ref[main:main.anonymous.fun2.closure], %b: i32): i32 {
    entry:
        %0 = LoadEffectField i32 ref[main:main.anonymous.fun2.closure] %callee <main:main.anonymous.fun2.closure::a>
        %1 = Add i32 i32 %0, i32 1
        %2 = StoreEffectField ref[main:main.anonymous.fun2.closure] ref[main:main.anonymous.fun2.closure] %callee, i32 %1 <main:main.anonymous.fun2.closure::a>
        %3 = LoadEffectField i32 ref[main:main.anonymous.fun2.closure] %2 <main:main.anonymous.fun2.closure::a>
        %4 = Add i32 i32 %b, i32 %3
        Ret void i32 %4
    } // anonymous.fun2.closure.apply

} // @main:main
)";
    //printf("%s\n", buf.c_str());
    ASSERT_EQ(z, buf);
}

TEST_F(IntermediateRepresentationGeneratorTest, StringTemplate) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
    IRGen("tests/31-ir-gen-string-template", &modules, &ok);
    ASSERT_TRUE(ok);
    
    ASSERT_TRUE(modules.find("yalx/lang:lang") != modules.end());
    ASSERT_TRUE(modules.find("main:main") != modules.end());
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    modules["main:main"]->PrintTo(&printer);
    
    static const char z[] = R"(module main @main:main {
source-files:
    [0] tests/31-ir-gen-string-template/src/main/main.yalx

globals:
    %main:main.x = GlobalValue f32 <"main:main.x">
    %main:main.y = GlobalValue f32 <"main:main.y">
    %main:main.s0 = GlobalValue string <"main:main.s0">

functions:
    fun $init(): void {
    boot:
        %0 = LoadFunAddr val[fun ()->void]* <fun foo:foo.$init>
        CallRuntime void val[fun ()->void]* %0, string "foo:foo" <PkgInitOnce>
        %1 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %1, string "yalx/lang:lang" <PkgInitOnce>
        StoreGlobal void f32 %main:main.x, f32 0.100000
        StoreGlobal void f32 %main:main.y, f32 0.200000
        %2 = LoadGlobal f32 f32 %main:main.x
        %3 = CallHandle string f32 %2 <f32::f32ToString>
        %4 = LoadGlobal f32 f32 %main:main.y
        %5 = CallHandle string f32 %4 <f32::f32ToString>
        %6 = Concat string string "", string %3, string ".", string %5
        StoreGlobal void string %main:main.s0, string %6
        Ret void
    } // main:main.$init

    fun issue01_string_concat_sanity(%x: f32, %y: f32): string {
    entry:
        %0 = CallHandle string f32 %x <f32::f32ToString>
        %1 = CallHandle string f32 %y <f32::f32ToString>
        %2 = Concat string string "", string %0, string ".", string %1
        Ret void string %2
    } // main:main.issue01_string_concat_sanity

    fun issue02_enum_concat(%foo: val[foo:foo.Foo]): string {
    entry:
        %0 = CallHandle string val[foo:foo.Foo] %foo <foo:foo.Foo::toString>
        %1 = Concat string string "foo=", string %0
        Ret void string %1
    } // main:main.issue02_enum_concat

    fun issue03_class_concat(%bar: ref[foo:foo.Bar]): string {
    entry:
        %0 = CallHandle string ref[foo:foo.Bar] %bar <foo:foo.Bar::toString>
        %1 = Concat string string "bar=", string %0
        Ret void string %1
    } // main:main.issue03_class_concat

    fun issue04_struct_concat(%baz: val[foo:foo.Baz]): string {
    entry:
        %0 = CallHandle string val[foo:foo.Baz] %baz <foo:foo.Baz::toString>
        %1 = Concat string string "baz=", string %0
        Ret void string %1
    } // main:main.issue04_struct_concat

    fun issue05_types_concat(%s: string, %a: i8, %b: u8, %c: i16, %d: u16): string {
    entry:
        %0 = CallHandle string i8 %a <i8::i8ToString>
        %1 = CallHandle string u8 %b <u8::u8ToString>
        %2 = CallHandle string i16 %c <i16::i16ToString>
        %3 = CallHandle string u16 %d <u16::u16ToString>
        %4 = Concat string string "s=", string %s, string ",a=", string %0, string ",b=", string %1, string ",c=", string %2, string ",d=", string %3
        Ret void string %4
    } // main:main.issue05_types_concat

    fun issue06_types_concat(%a: i32, %b: u32, %c: i64, %d: u64, %e: u8): string {
    entry:
        %0 = CallHandle string i32 %a <i32::i32ToString>
        %1 = CallHandle string u32 %b <u32::u32ToString>
        %2 = CallHandle string i64 %c <i64::i64ToString>
        %3 = CallHandle string u64 %d <u64::u64ToString>
        %4 = CallHandle string u8 %e <u8::u8ToString>
        %5 = Concat string string "a=", string %0, string ",b=", string %1, string ",c=", string %2, string ",d=", string %3, string ",e=", string %4
        Ret void string %5
    } // main:main.issue06_types_concat

} // @main:main
)";
    //printf("%s\n", buf.c_str());
    ASSERT_EQ(z, buf);
}

} // namespace ir

} // namespace yalx
