#include "base/io.h"
#include <gtest/gtest.h>

namespace yalx::base {

TEST(IoTest, Sanity) {
    std::string buf;
    PrintingWriter p(NewMemoryWritableFile(&buf), true);
    p.Println("%s=%d", "i", 100);
    p.Writeln("ok");
    ASSERT_EQ("i=100\nok\n", buf);
}

} // namespace yalx
