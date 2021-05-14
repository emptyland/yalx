#include "base/env.h"
#include "base/io.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace yalx {

namespace base {

class EnvTest : public ::testing::Test {
public:
    
    void TearDown() override {
        for (int i = 0; kCleanupFiles[i] != nullptr; i++) {
            std::filesystem::path path(kCleanupFiles[i]);
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        }
    }
    
    static const char *kCleanupFiles[];
};

const char *EnvTest::kCleanupFiles[] = {
    "tests/.not_exists_file",
    "tests/write-stub-file",
    "tests/write-read-stub-file",
    nullptr
};

TEST_F(EnvTest, OSPageMemory) {
    auto memory = Env::OSPageAllocate(Env::kOSPageSize, Env::kMemoryWriteable|Env::kMemoryExecuteable);
    ASSERT_TRUE(memory.is_valid());
    ASSERT_EQ(Env::kOSPageSize, memory.size());
}

TEST_F(EnvTest, OpenNotExistFile) {
    std::unique_ptr<SequentialFile> file;
    auto rs = Env::NewSequentialFile("tests/.not_exists_file", &file);
    ASSERT_TRUE(rs.IsCorruption());
    //FAIL() << rs.ToString();
}

TEST_F(EnvTest, FileExist) {
    std::filesystem::path path("tests/.not_exists_file");
    ASSERT_FALSE(std::filesystem::exists(path));
}

TEST_F(EnvTest, WriteFile) {
    std::unique_ptr<WritableFile> file;
    auto rs = Env::NewWritableFile("tests/write-stub-file", false, &file);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    rs = file->Append("hello\n");
    ASSERT_TRUE(rs.ok()) << rs.ToString();

    rs = file->Append("world\n");
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    file.reset();
    
    std::filesystem::path path("tests/write-stub-file");
    ASSERT_TRUE(std::filesystem::exists(path));
}

TEST_F(EnvTest, ReadFile) {
    static const char kFileName[] = "tests/write-read-stub-file";
    
    std::unique_ptr<WritableFile> file;
    auto rs = Env::NewWritableFile(kFileName, false, &file);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    file->Append("hello\nworld\n");
    file.reset();
    
    std::unique_ptr<SequentialFile> rd;
    rs = Env::NewSequentialFile(kFileName, &rd);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    size_t n;
    rs = rd->Available(&n);
    ASSERT_EQ(12, n);
    std::string_view result;
    std::string scratch;
    rs = rd->Read(n, &result, &scratch);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    ASSERT_EQ("hello\nworld\n", result);
}

TEST_F(EnvTest, MemoryReadFile) {
    std::unique_ptr<SequentialFile> file(NewMemorySequentialFile("hello\nworld\n"));
    size_t n;
    auto rs = file->Available(&n);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    ASSERT_EQ(12, n);
    
    std::string_view result;
    rs = file->Read(n, &result, nullptr);
    ASSERT_TRUE(rs.ok()) << rs.ToString();
    
    ASSERT_EQ("hello\nworld\n", result);
}


} // namespace base

} // namespace yalx
