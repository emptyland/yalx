#include "ir/source-position.h"
#include "compiler/source-position.h"
#include <gtest/gtest.h>

namespace yalx {

namespace ir {

class SourcePositionTest : public ::testing::Test {
public:
    SourcePositionTest(): source_position_table_(&arena_) {}
    
protected:
    base::Arena arena_;
    SourcePositionTable source_position_table_;
}; // class SourcePositionTest

TEST_F(SourcePositionTest, Sanity) {
    auto unknown = SourcePosition::Unknown();
    EXPECT_EQ(-1, unknown.line());
    EXPECT_EQ(-1, unknown.column());
}

TEST_F(SourcePositionTest, SourcePositionTable) {
    auto file_name = base::ArenaString::New(&arena_, "main.yalx");
    auto file_id = source_position_table_.FindOrInsertFileName(file_name);
    EXPECT_EQ(0, file_id);
    EXPECT_EQ(file_id, source_position_table_.FindOrInsertFileName(file_name));
    
    file_name = base::ArenaString::New(&arena_, "foo.yalx");
    auto file_id2 = source_position_table_.FindOrInsertFileName(file_name);
    EXPECT_EQ(1, file_id2);
    EXPECT_NE(file_id, file_id2);
}

TEST_F(SourcePositionTest, SourcePositionTableScope) {
    auto file_name = base::ArenaString::New(&arena_, "main.yalx");
    SourcePositionTable::Scope scope(file_name, {1,2}, &source_position_table_);
    auto location = scope.Position();
    EXPECT_EQ(0, location.file_id());
    EXPECT_EQ(1, location.line());
    EXPECT_EQ(2, location.column());
}

} // namespace ir

} // namespace yalx
