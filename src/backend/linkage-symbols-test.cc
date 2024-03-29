#include "backend/linkage-symbols.h"
#include <gtest/gtest.h>

namespace yalx::backend {

class LinkageSymbolsTest : public ::testing::Test {
public:
    LinkageSymbolsTest(): symbols_(&arena_) {}
    
protected:
    base::Arena arena_;
    Linkage symbols_;
}; // class LinkageSymbolsTest

#ifdef YALX_OS_DARWIN
TEST_F(LinkageSymbolsTest, Sanity) {
    auto symbol = symbols_.Mangle("main:main.$init");
    ASSERT_TRUE(nullptr != symbol);
    ASSERT_STREQ("_main_Zomain_Zd_Z4init", symbol->data());
    
    symbol = symbols_.Mangle("yalx/lang:lang.Any.hashCode");
    ASSERT_STREQ("_yalx_Zplang_Zolang_ZdAny_ZdhashCode", symbol->data());
    
    symbol = symbols_.Mangle("yalx/lang:lang.Number<i32>");
    ASSERT_STREQ("_yalx_Zplang_Zolang_ZdNumber_Dki32_Dl", symbol->data());
    
    ASSERT_STREQ("_memcpy", kLibc_memcpy->data());
}
#endif

// _yalx_Zplang_Zolang_ZdAny_ZdAny_Z4constructor
// _yalx_Zplang_Zolang_ZdAny_Zdfinalize
// _yalx_Zplang_Zolang_ZdAny_ZdhashCode
// _yalx_Zplang_Zolang_ZdAny_Zdid
// _yalx_Zplang_Zolang_ZdAny_ZdisEmpty
// _yalx_Zplang_Zolang_Zd_Z4init

} // namespace yalx
