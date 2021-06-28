#include "compiler/parser.h"
#include "compiler/syntax-feedback.h"
#include "compiler/lexer.h"
#include "compiler/ast.h"
#include "base/env.h"
#include "base/io.h"
#include "base/arena.h"
#include <gtest/gtest.h>


namespace yalx {

namespace cpl {

class ParserTest : public ::testing::Test {
public:
    class MockErrorFeedback : public SyntaxFeedback {
    public:
        void DidFeedback(const SourcePosition &location, const char *z, size_t n) override {
            ::printf("[%s:%d] %s\n", file_name().data(), location.begin_line(), z);
        }
        
        void DidFeedback(const char *z) override {
            ::puts(z);
        }
    }; // class MockErrorFeedback
    
    ParserTest(): parser_(&arena_, &feedback_) {}
    
    void SetUp() override {}
    void TearDown() override {}
    
    void SwitchInput(const char *z) {
        auto file = base::NewMemorySequentialFile(z, strlen(z));
        auto rs = parser_.SwitchInputFile(":memory:", file);
        ASSERT_TRUE(rs.ok()) << rs.ToString();
    }
protected:
    base::Arena arena_;
    MockErrorFeedback feedback_;
    Parser parser_;
    
}; // class ParserTest



TEST_F(ParserTest, Sanity) {
    SwitchInput("package main\n"
                "\n"
                "import \"foo/bar\"");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, file_unit);
    
    ASSERT_NE(nullptr, file_unit->package_name());
    EXPECT_STREQ("main", file_unit->package_name()->data());
    
    ASSERT_EQ(1, file_unit->imports_size());
    EXPECT_STREQ("foo/bar", file_unit->import(0)->package_path()->data());
}

TEST_F(ParserTest, ImportStatements) {
    SwitchInput("package main\n"
                "\n"
                "import \"foo/bar\" as fb\n"
                "import \"bar/baz\" as *\n");
    bool ok = true;
    auto file_unit = parser_.Parse(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, file_unit);
    
    ASSERT_EQ(2, file_unit->imports_size());
    EXPECT_STREQ("foo/bar", file_unit->import(0)->package_path()->data());
    EXPECT_STREQ("fb", file_unit->import(0)->alias()->data());
    EXPECT_STREQ("bar/baz", file_unit->import(1)->package_path()->data());
    EXPECT_STREQ("*", file_unit->import(1)->alias()->data());
}

TEST_F(ParserTest, Annotation) {
    SwitchInput("@Foo\n"
                "@foo.Bar\n"
                "@Foo(id=1)\n"
                "@Bar(name=\"John\")\n");
    bool ok = true;
    auto annotation = parser_.ParseAnnotation(false /*skip_at*/, &ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, annotation);
    
    ASSERT_STREQ("Foo", annotation->name()->name()->data());
    EXPECT_EQ(nullptr, annotation->name()->prefix_name());
    
    annotation = parser_.ParseAnnotation(false /*skip_at*/, &ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, annotation);
    
    EXPECT_STREQ("foo", annotation->name()->prefix_name()->data());
    EXPECT_STREQ("Bar", annotation->name()->name()->data());
    
    annotation = parser_.ParseAnnotation(false /*skip_at*/, &ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, annotation);
    
    EXPECT_STREQ("Foo", annotation->name()->name()->data());
    EXPECT_EQ(nullptr, annotation->name()->prefix_name());
    ASSERT_EQ(1, annotation->fields_size());
    auto field = annotation->field(0);
    auto value = static_cast<IntLiteral *>(field->value());
    EXPECT_STREQ("id", field->name()->data());
    ASSERT_EQ(Node::kIntLiteral, value->kind());
    EXPECT_EQ(1, value->value());
}

TEST_F(ParserTest, AnnotationArray) {
    SwitchInput("@Foo(vars={1,2,3})\n");
    bool ok = true;
    auto annotation = parser_.ParseAnnotation(false /*skip_at*/, &ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, annotation);
}

TEST_F(ParserTest, AnnotationDeclaration) {
    SwitchInput("@Foo\n"
                "@foo.Bar(foo=Foo(id=1))\n"
                "@Foo(id=1)\n"
                "@Bar(name=\"John\")\n");
    bool ok = true;
    auto decl = parser_.ParseAnnotationDeclaration(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, decl);
    
    ASSERT_EQ(4, decl->annotations_size());
    auto anno = decl->annotation(1);
    auto field = anno->field(0);
    EXPECT_STREQ("foo", field->name()->data());
    ASSERT_TRUE(field->IsNested());
    anno = field->nested();
    EXPECT_STREQ("Foo", anno->name()->name()->data());
    ASSERT_EQ(1, anno->fields_size());
    field = anno->field(0);
    EXPECT_STREQ("id", field->name()->data());
    ASSERT_TRUE(field->IsValue());
    ASSERT_TRUE(field->value()->IsIntLiteral());
    EXPECT_EQ(1, static_cast<IntLiteral *>(field->value())->value());
    
    
}

TEST_F(ParserTest, SimpleFunctionPrototype) {
    SwitchInput("()->unit\n");
    bool ok = true;
    auto ast = parser_.ParseType(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsFunctionPrototype());
    auto fun = ast->AsFunctionPrototype();
    EXPECT_FALSE(fun->vargs());
    ASSERT_EQ(0, fun->params_size());
    ASSERT_EQ(1, fun->return_types_size());
    EXPECT_EQ(Type::kType_unit, fun->return_type(0)->primary_type());
}

TEST_F(ParserTest, MutliParamsFunctionPrototype) {
    SwitchInput("(int, string, ...)->unit\n");
    bool ok = true;
    auto ast = parser_.ParseType(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsFunctionPrototype());
    auto fun = ast->AsFunctionPrototype();
    ASSERT_EQ(2, fun->params_size());
    EXPECT_EQ(Type::kType_i32, fun->param(0)->AsType()->primary_type());
    EXPECT_EQ(Type::kType_string, fun->param(1)->AsType()->primary_type());
    EXPECT_TRUE(fun->vargs());
    ASSERT_EQ(1, fun->return_types_size());
    EXPECT_EQ(Type::kType_unit, fun->return_type(0)->primary_type());
}

TEST_F(ParserTest, MutliReturnsFunctionPrototype) {
    SwitchInput("(...)->(i8[],i8,i16)\n");
    bool ok = true;
    auto ast = parser_.ParseType(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsFunctionPrototype());
    auto fun = ast->AsFunctionPrototype();
    ASSERT_EQ(0, fun->params_size());
    EXPECT_TRUE(fun->vargs());
    ASSERT_EQ(3, fun->return_types_size());
    EXPECT_EQ(Type::kArray, fun->return_type(0)->category());
    EXPECT_EQ(Type::kType_i8, fun->return_type(0)->generic_arg(0)->primary_type());
    EXPECT_EQ(Type::kType_i8, fun->return_type(1)->primary_type());
    EXPECT_EQ(Type::kType_i16, fun->return_type(2)->primary_type());
}

} // namespace cpl

} // namespace yalx
