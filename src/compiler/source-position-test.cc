#include "compiler/source-position.h"
#include <gtest/gtest.h>


namespace yalx::cpl {

    TEST(SourcePosition, Sanity) {
        SourcePosition loc(1, 1);
        ASSERT_EQ(1, loc.begin_line());
        ASSERT_EQ(1, loc.begin_column());
        ASSERT_EQ(1, loc.end_line());
        ASSERT_EQ(1, loc.end_column());
    }

    TEST(SourcePosition, Initializer) {
        SourcePosition loc(1, 2, 3, 4);
        ASSERT_EQ(1, loc.begin_line());
        ASSERT_EQ(2, loc.begin_column());
        ASSERT_EQ(3, loc.end_line());
        ASSERT_EQ(4, loc.end_column());
    }

} // namespace yalx
