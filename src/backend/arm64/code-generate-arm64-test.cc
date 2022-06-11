#include "backend/x64/code-generate-x64.h"
#include "backend/code-generate-arch-test.h"
#include "backend/constants-pool.h"
#include "backend/linkage-symbols.h"
#include "backend/instruction.h"
#include "compiler/compiler.h"
#include "ir/base-test.h"
#include "ir/node.h"
#include "base/io.h"
#include "runtime/object/yalx-string.h"
#include "runtime/object/type.h"
#include "runtime/heap/heap.h"
#include "runtime/process.h"
#include "runtime/runtime.h"
#include <gtest/gtest.h>

namespace yalx {
namespace backend {

class Arm64CodeGeneratorTest : public ir::BaseTest {
public:
    Arm64CodeGeneratorTest()
    : const_pool_(arena())
    , symbols_(arena()) {}

    void CodeGen(const char *project_dir, std::string_view name, base::PrintingWriter *printer, bool *ok) {
        base::ArenaMap<std::string_view, ir::Module *> modules(arena());
        IRGen(project_dir, &modules, ok);
        if (!*ok) {
            return;
        }
        ASSERT_TRUE(modules.find(name) != modules.end());
        //modules[name]->PrintTo(printer);
        
        base::ArenaMap<std::string_view, backend::InstructionFunction *> funs(arena());
        cpl::Compiler::SelectArm64InstructionCode(arena(), modules[name], &const_pool_, &symbols_, 1, &funs);
        cpl::Compiler::GenerateArm64InstructionCode(funs, modules[name], &const_pool_, &symbols_, printer);
    }
protected:
    ConstantsPool const_pool_;
    LinkageSymbols symbols_;
}; // class Arm64CodeGeneratorTest

TEST_F(Arm64CodeGeneratorTest, Sanity) {
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/40-code-gen-sanity", "main:main", &printer, &ok);
    ASSERT_TRUE(ok);
    //printf("%s\n", buf.c_str());
}

//TEST_F(Arm64CodeGeneratorTest, StructsGenerating) {
//    std::string buf;
//    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
//    bool ok = true;
//    CodeGen("tests/41-code-gen-structs", "issue02:issue02", &printer, &ok);
//    ASSERT_TRUE(ok);
//    //printf("%s\n", buf.c_str());
//}

TEST_F(Arm64CodeGeneratorTest, YalxLang) {
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/40-code-gen-sanity", "yalx/lang:lang", &printer, &ok);
    ASSERT_TRUE(ok);
    //printf("%s\n", buf.c_str());
}

TEST_F(Arm64CodeGeneratorTest, TryCatch) {
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/25-ir-throw-catch-expr", "issue00:issue00", &printer, &ok);
    ASSERT_TRUE(ok);
    //printf("%s\n", buf.c_str());
}

TEST_F(Arm64CodeGeneratorTest, FooMetadata) {
    
    auto clazz = yalx_find_class("issue02:issue02.Foo");
    ASSERT_TRUE(clazz != NULL);
    ASSERT_EQ(3, clazz->n_fields);
    ASSERT_STREQ("x", clazz->fields[0].name.z);
    ASSERT_STREQ("y", clazz->fields[1].name.z);
    ASSERT_STREQ("name", clazz->fields[2].name.z);
    
    ASSERT_STREQ("doIt", clazz->methods[0].name.z);
    ASSERT_STREQ("fun (issue02:issue02.Foo)->(void)", clazz->methods[0].prototype_desc.z);

    ASSERT_STREQ("doThat", clazz->methods[1].name.z);
    
}

#ifdef YALX_ARCH_ARM64

TEST_F(Arm64CodeGeneratorTest, ReturningVals) {
    int buf[4] = {0};
    call_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zdissue1));
    ASSERT_EQ(3, buf[3]);
    memset(buf, 0, sizeof(buf));
    call_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zdfoo));
    ASSERT_EQ(1066192077, buf[1]);
    ASSERT_EQ(2, buf[2]);
    ASSERT_EQ(1, buf[3]);
    memset(buf, 0, sizeof(buf));
    call_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zdissue5));
    ASSERT_EQ(4, buf[0]);
    ASSERT_EQ(3, buf[1]);
    ASSERT_EQ(2, buf[2]);
    ASSERT_EQ(1, buf[3]);
}

TEST_F(Arm64CodeGeneratorTest, PkgInitOnce) {
//    int buf[4] = {0};
//    call_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zd_Z4init));
    pkg_init_once(reinterpret_cast<void *>(&main_Zomain_Zd_Z4init), "main:main");
}

// issue9_stub
TEST_F(Arm64CodeGeneratorTest, CallNativeHandle) {
    yalx_returning_vals state;
    yalx_enter_returning_scope(&state, 16, nullptr);
    auto vals = reinterpret_cast<int *>(state.buf);
    
    main_Zomain_Zdmain_had();

    main_Zomain_Zdissue6_had(1, 2);
    ASSERT_EQ(-1, vals[3]);
    
    main_Zomain_Zdissue6_had(2, 1);
    ASSERT_EQ(4, vals[3]);
    
    yalx_exit_returning_scope(&state);
}

TEST_F(Arm64CodeGeneratorTest, StackAndHeapAllocStruct) {
    pkg_init_once(reinterpret_cast<void *>(&issue02_Zoissue02_Zd_Z4init), "issue02:issue02");
    issue02_Zoissue02_Zdissue1_had();
    issue02_Zoissue02_Zdissue2_had();
    issue02_Zoissue02_Zdissue3_had();
}

TEST_F(Arm64CodeGeneratorTest, TryCatchSanity) {
    pkg_init_once(reinterpret_cast<void *>(&issue00_Zoissue00_Zd_Z4init), "issue00:issue00");
    issue00_Zoissue00_Zdissue1_had();
}

#endif // YALX_ARCH_ARM64

} // namespace backend
} // namespace yalx
