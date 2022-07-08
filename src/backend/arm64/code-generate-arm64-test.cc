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
#include "runtime/object/arrays.h"
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

TEST_F(Arm64CodeGeneratorTest, StructsGenerating) {
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/41-code-gen-structs", "issue02:issue02", &printer, &ok);
    ASSERT_TRUE(ok);
    //printf("%s\n", buf.c_str());
}

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

TEST_F(Arm64CodeGeneratorTest, GlobalVars) {
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/42-code-gen-globals", "issue03:issue03", &printer, &ok);
    ASSERT_TRUE(ok);
    //printf("%s\n", buf.c_str());
    
}

TEST_F(Arm64CodeGeneratorTest, ArrayInitAndAlloc) {
    
    std::string buf;
    base::PrintingWriter printer(base::NewMemoryWritableFile(&buf), true/*ownership*/);
    bool ok = true;
    CodeGen("tests/43-code-gen-arrays", "issue04:issue04", &printer, &ok);
    ASSERT_TRUE(ok);
    printf("%s\n", buf.c_str());
    
}

// #ifdef YALX_ARCH_ARM64

TEST_F(Arm64CodeGeneratorTest, ReturningVals) {
    int buf[4] = {0};
    call0_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zdissue1));
    ASSERT_EQ(3, buf[3]);
    memset(buf, 0, sizeof(buf));
    call0_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zdfoo));
    ASSERT_EQ(1066192077, buf[1]);
    ASSERT_EQ(2, buf[2]);
    ASSERT_EQ(1, buf[3]);
    memset(buf, 0, sizeof(buf));
    call0_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zdissue5));
    ASSERT_EQ(4, buf[0]);
    ASSERT_EQ(3, buf[1]);
    ASSERT_EQ(2, buf[2]);
    ASSERT_EQ(1, buf[3]);
}

TEST_F(Arm64CodeGeneratorTest, PkgInitOnce) {
//    int buf[4] = {0};
//    call0_returning_vals(buf, sizeof(buf), reinterpret_cast<void *>(&main_Zomain_Zd_Z4init));
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

TEST_F(Arm64CodeGeneratorTest, GlobalInit03) {
    pkg_init_once(reinterpret_cast<void *>(&issue03_Zoissue03_Zd_Z4init), "issue03:issue03");
    auto slots = pkg_get_global_slots("issue03:issue03");
    ASSERT_NE(nullptr, slots);
    ASSERT_GT(slots->mark_size, 0);
    auto obj = *reinterpret_cast<yalx_ref_t *>(slots->slots + slots->marks[0]);
    ASSERT_STREQ("Foo", CLASS(obj)->name.z);
    
    auto s1 = reinterpret_cast<yalx_str_handle>(slots->slots + slots->marks[1]);
    ASSERT_STREQ("Hello", yalx_str_bytes(s1));
    
    auto s2 = reinterpret_cast<yalx_str_handle>(slots->slots + slots->marks[2]);
    ASSERT_STREQ("World", yalx_str_bytes(s2));
    //printd("%s|%s", CLASS(obj)->name.z, CLASS(obj)->location.z);
    
    yalx_returning_vals state;
    yalx_enter_returning_scope(&state, 16, nullptr);
    auto vals = reinterpret_cast<int *>(state.buf);

    issue03_Zoissue03_Zdissue1_had();
    ASSERT_EQ(777, vals[2]);
    ASSERT_EQ(996, vals[3]);

    yalx_exit_returning_scope(&state);
}

struct Issue04DummyFoo {
    YALX_VALUE_HEADER;
    i32_t i;
    yalx_value_str *name;
};

static_assert(32 == sizeof(Issue04DummyFoo), "");

TEST_F(Arm64CodeGeneratorTest, ArrayInitialization) {
    pkg_init_once(reinterpret_cast<void *>(&issue04_Zoissue04_Zd_Z4init), "issue04:issue04");
    
    yalx_returning_vals state;
    yalx_enter_returning_scope(&state, 16, nullptr);

    issue04_Zoissue04_Zdissue1_had();

    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("Array", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_array *>(ref);
        ASSERT_EQ(&builtin_classes[Type_i32], ar->item);
        ASSERT_EQ(4, ar->len);
        auto elements = reinterpret_cast<const int *>(ar->data);
        EXPECT_EQ(1, elements[0]);
        EXPECT_EQ(2, elements[1]);
        EXPECT_EQ(3, elements[2]);
        EXPECT_EQ(4, elements[3]);
    }
    yalx_exit_returning_scope(&state);
    
    yalx_enter_returning_scope(&state, 16, nullptr);

    issue04_Zoissue04_Zdissue2_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("Array", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_array *>(ref);
        ASSERT_STREQ("String", ar->item->name.z);
        ASSERT_EQ(3, ar->len);
        auto elements = reinterpret_cast<yalx_value_str **>(ar->data);
        EXPECT_STREQ("hello", elements[0]->bytes);
        EXPECT_STREQ("world", elements[1]->bytes);
        EXPECT_STREQ("!", elements[2]->bytes);
    }
    yalx_exit_returning_scope(&state);
    
    
    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue3_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("Array", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_array *>(ref);
        ASSERT_STREQ("Array", ar->item->name.z);
        ASSERT_EQ(3, ar->len);
        auto elements = reinterpret_cast<yalx_value_array **>(ar->data);
        USE(elements);
    }
    yalx_exit_returning_scope(&state);
    
    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue4_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("Array", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_array *>(ref);
        ASSERT_STREQ("Array", ar->item->name.z);
        ASSERT_EQ(10, ar->len);
        auto elements = reinterpret_cast<yalx_value_array **>(ar->data);
        for (int i = 0; i < ar->len; i++) {
            ASSERT_EQ(8, elements[i]->len);
        }
    }
    yalx_exit_returning_scope(&state);
    
    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue7_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("Array", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_array *>(ref);
        ASSERT_STREQ("Foo", ar->item->name.z);
        ASSERT_STREQ("issue04:issue04.Foo", ar->item->location.z);
        ASSERT_EQ(32, ar->item->instance_size);
        ASSERT_EQ(2, ar->len);
        
        auto elems = reinterpret_cast<Issue04DummyFoo *>(ar->data);
        ASSERT_EQ(996, elems[0].i);
        ASSERT_STREQ("hello", elems[0].name->bytes);
        ASSERT_EQ(700, elems[1].i);
        ASSERT_STREQ("ok", elems[1].name->bytes);
    }
    yalx_exit_returning_scope(&state);

    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue8_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("MultiDimsArray", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_multi_dims_array *>(ref);
        ASSERT_STREQ("i32", ar->item->name.z);
        ASSERT_EQ(6, ar->len);
        ASSERT_EQ(2, ar->dims);
        ASSERT_EQ(3, ar->caps[0]);
        ASSERT_EQ(2, ar->caps[1]);
        
        auto elems = reinterpret_cast<const int *>(yalx_multi_dims_array_data(ar));
        for (int i = 0; i < ar->len; i++) {
            EXPECT_EQ(i + 1, elems[i]);
        }
        auto n = 1;
        for (int x = 0; x < ar->caps[0]; x++) {
            for (int y = 0; y < ar->caps[1]; y++) {
                ASSERT_EQ(n++, *static_cast<int *>(yalx_array_location2(ar, x, y)));
            }
        }
    }
    yalx_exit_returning_scope(&state);
    
    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue9_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("MultiDimsArray", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_multi_dims_array *>(ref);
        ASSERT_STREQ("u8", ar->item->name.z);
        ASSERT_EQ(3, ar->len);
        ASSERT_EQ(2, ar->dims);
        ASSERT_EQ(1, ar->caps[0]);
        ASSERT_EQ(3, ar->caps[1]);
        
        ASSERT_EQ(9, *static_cast<u8_t *>(yalx_array_location2(ar, 0, 0)));
        ASSERT_EQ(8, *static_cast<u8_t *>(yalx_array_location2(ar, 0, 1)));
        ASSERT_EQ(7, *static_cast<u8_t *>(yalx_array_location2(ar, 0, 2)));
    }
    yalx_exit_returning_scope(&state);
    
    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue10_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("MultiDimsArray", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_multi_dims_array *>(ref);
        ASSERT_STREQ("f32", ar->item->name.z);
        ASSERT_EQ(4, ar->len);
        ASSERT_EQ(2, ar->dims);
        ASSERT_EQ(2, ar->caps[0]);
        ASSERT_EQ(2, ar->caps[1]);
        
        ASSERT_NEAR(1.1f, *static_cast<f32_t *>(yalx_array_location2(ar, 0, 0)), 0.01f);
        ASSERT_NEAR(1.2f, *static_cast<f32_t *>(yalx_array_location2(ar, 0, 1)), 0.01f);
        ASSERT_NEAR(1.3f, *static_cast<f32_t *>(yalx_array_location2(ar, 1, 0)), 0.01f);
        ASSERT_NEAR(1.4f, *static_cast<f32_t *>(yalx_array_location2(ar, 1, 1)), 0.01f);
    }
    yalx_exit_returning_scope(&state);
    
    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue11_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("MultiDimsArray", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_multi_dims_array *>(ref);
        ASSERT_STREQ("i16", ar->item->name.z);
        ASSERT_EQ(64, ar->len);
        ASSERT_EQ(3, ar->dims);
        ASSERT_EQ(4, ar->caps[0]);
        ASSERT_EQ(4, ar->caps[1]);
        ASSERT_EQ(4, ar->caps[2]);
        
        auto elems = reinterpret_cast<const i16_t *>(yalx_multi_dims_array_data(ar));
        for (int i = 0; i < ar->len; i++) {
            ASSERT_EQ(99, elems[i]) << "i=" << i;
        }
    }
    yalx_exit_returning_scope(&state);

    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue5_had();
    
    {
        auto vals = reinterpret_cast<int *>(state.buf);
        ASSERT_EQ(2, vals[3]);
        ASSERT_EQ(4, vals[2]);
        ASSERT_EQ(6, vals[1]);
    }
    yalx_exit_returning_scope(&state);
    
    yalx_enter_returning_scope(&state, 32, nullptr);
    
    issue04_Zoissue04_Zdissue12_had();
    
    {
        auto vals = reinterpret_cast<yalx_str_handle>(state.buf);
        ASSERT_STREQ("a", vals[3]->bytes);
        ASSERT_STREQ("b", vals[2]->bytes);
        ASSERT_STREQ("d", vals[1]->bytes);
    }
    yalx_exit_returning_scope(&state);

#if 0
    yalx_enter_returning_scope(&state, 16, nullptr);
    
    issue04_Zoissue04_Zdissue6_had();
    
    {
        auto vals = reinterpret_cast<yalx_ref_t *>(state.buf);
        auto ref = vals[1];
        ASSERT_NE(nullptr, ref);
        auto klass = CLASS(ref);
        ASSERT_NE(nullptr, klass);
        ASSERT_STREQ("TypedArray", klass->name.z);
        
        auto ar = reinterpret_cast<yalx_value_array *>(ref);
        ASSERT_STREQ("Foo", ar->item->name.z);
        ASSERT_STREQ("issue04:issue04.Foo", ar->item->location.z);
        ASSERT_EQ(ar->len, 10);
        
        struct FooDummy {
            uintptr_t padding0;
            uintptr_t padding1;
            i32_t i;
            yalx_value_str *name;
        };
        
        FooDummy *foo = ((FooDummy *)ar->data);
        ASSERT_EQ(0, foo[0].i);
        ASSERT_EQ(0, foo[0].name->len);
        
        ASSERT_EQ(1, foo[1].i);
        ASSERT_EQ(2, foo[1].name->len);
        ASSERT_STREQ("ok", foo[1].name->bytes);
        
        ASSERT_EQ(0, foo[2].i);
        ASSERT_EQ(0, foo[2].name->len);
    }
    yalx_exit_returning_scope(&state);
#endif
}

//#endif // YALX_ARCH_ARM64

} // namespace backend
} // namespace yalx
