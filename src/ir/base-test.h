#pragma once
#ifndef YALX_IR_BASE_TEST_H_
#define YALX_IR_BASE_TEST_H_

#include "compiler/syntax-feedback.h"
#include "base/arena-utils.h"
#include "base/base.h"
#include <gtest/gtest.h>

namespace yalx {
namespace ir {

class OperatorsFactory;
class Module;

class BaseTest : public ::testing::Test {
public:
    class MockErrorFeedback;
    
    BaseTest();
    ~BaseTest() override;
    
    base::Arena *arena() { return &arean_; }
    DEF_PTR_GETTER(OperatorsFactory, ops);
    DEF_PTR_GETTER(cpl::SyntaxFeedback, feedback);
    
    void IRGen(const char *project_dir, base::ArenaMap<std::string_view, Module *> *modules, bool *ok);
    
protected:
    base::Arena ast_arean_;
    base::Arena arean_;
    OperatorsFactory *ops_;
    cpl::SyntaxFeedback *feedback_;
}; // class BaseTest

} // namespace ir
} // namespace yalx

#endif // YALX_IR_BASE_TEST_H_
