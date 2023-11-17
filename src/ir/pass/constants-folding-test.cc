#include "ir/pass/constants-folding.h"
#include "ir/codegen.h"
#include "ir/metadata.h"
#include "ir/node.h"
#include "ir/operators-factory.h"
#include "ir/base-test.h"
#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class ConstantsFoldingPassTest : public BaseTest {
public:
    void RunPass(ConstantsFoldingPass::ModulesMap *modules) {
        ConstantsFoldingPass(arena(), ops(), modules, feedback()).Run();
    }
}; // class ConstantsFoldingPassTest

// 24-ir-constants-folding
TEST_F(ConstantsFoldingPassTest, Sanity) {
    bool ok = false;
    base::ArenaMap<std::string_view, Module *> modules(&arena_);
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
        %0 = LoadFunAddr val[fun ()->void]* <fun yalx/lang:lang.$init>
        CallRuntime void val[fun ()->void]* %0, string "yalx/lang:lang" <PkgInitOnce>
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
        %0 = FCmp u8 f32 0.100000, f32 0.200000 <ogt>
        %1 = FCmp u8 f32 0.100000, f32 0.200000 <olt>
        Ret void u8 %0, u8 %1
    } // main:main.issue6

    fun main(): void {
    entry:
        Ret void
    } // main:main.main

} // @main:main
)";
    //printf("%s\n", buf.data());
    ASSERT_EQ(z, buf);
}

} // namespace ir

} // namespace yalx
