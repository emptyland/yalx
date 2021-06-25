#include "compiler/ast.h"
#include <gtest/gtest.h>

namespace yalx {

namespace cpl {

class AstTest : public ::testing::Test {
  
protected:
    base::Arena arena_;
};

TEST_F(AstTest, FileUnit) {
    auto file_name = String::New(&arena_, "demo.yalx");
    auto full_file_path = String::New(&arena_, "demo/demo.yalx");
    auto file_unit = new (&arena_) FileUnit(&arena_, file_name, full_file_path, {0, 0});
    EXPECT_STREQ("demo.yalx", file_unit->file_name()->data());
    EXPECT_STREQ("demo/demo.yalx", file_unit->file_full_path()->data());
    
    auto original_package_name = String::New(&arena_, "demo");
    auto package_path = String::New(&arena_, "demo");
    auto import = new (&arena_) FileUnit::ImportEntry(original_package_name, package_path, nullptr, {0, 0});
    file_unit->mutable_imports()->push_back(import);
    ASSERT_EQ(1, file_unit->imports_size());
    ASSERT_EQ(import, file_unit->import(0));
}

}

}
