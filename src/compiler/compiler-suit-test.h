#pragma once
#ifndef YALX_COMPILER_COMPILER_SUIT_TEST_H
#define YALX_COMPILER_COMPILER_SUIT_TEST_H

#include "compiler/compiler.h"
#include "compiler/syntax-feedback.h"
#include "compiler/source-position.h"
#include "compiler/ast.h"
#include "base/io.h"
#include "base/env.h"
#include "base/arena.h"
#include <gtest/gtest.h>

namespace yalx::cpl {

class CompilerTestSuit : public ::testing::Test {
public:
    class MockErrorFeedback : public SyntaxFeedback {
    public:
        void DidFeedback(const SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(),
                     location.begin_column(),
                     location.end_line(), location.end_column(), z);
        }

        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback

    CompilerTestSuit(): entries_(&arena_), all_(&arena_) {}

    void SetUp() override {}

    void TearDown() override {}

    base::Status ParseAll(const std::string &path) {
        MockErrorFeedback feedback;
        auto rs = Compiler::FindAndParseProjectSourceFiles(path, "libs", &arena_,
                                                           &feedback, &main_pkg_, &entries_,
                                                           &all_);
        return rs;
    }

    void PrintAllPackages() {
        for (auto [key, pkg] : all_) {
            printf("[%s] %s\n", key.data(), pkg->name()->data());
        }
        for (auto pkg : entries_) {
            printf("<%s> %s\n", pkg->name()->data(), pkg->path()->data());
        }
        printf("<%s> %s\n", main_pkg_->name()->data(), main_pkg_->path()->data());

        //main_pkg_->source_file()
    }

//    static std::vector<std::string> MakeSearchPaths(const char *project_name) {
//        std::vector<std::string> search_paths;
//        std::string prefix("tests");
//        prefix.append("/").append(project_name);
//        std::string path(prefix);
//        path.append("/src");
//        search_paths.push_back(path);
//        path.assign(prefix);
//        path.append("/pkg");
//        search_paths.push_back(path);
//        search_paths.emplace_back("libs");
//        return search_paths;
//    }

protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    Package *main_pkg_ = nullptr;
    base::ArenaVector<Package *> entries_;
    base::ArenaMap<std::string_view, Package *> all_;
}; // class ParserTest

}

#endif // YALX_COMPILER_COMPILER_SUIT_TEST_H
