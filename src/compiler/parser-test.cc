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
            ::printf("[%s:(%d,%d)-(%d,%d)] %s\n", file_name().data(), location.begin_line(), location.begin_column(),
                     location.end_line(), location.end_column(), z);
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

TEST_F(ParserTest, SimpleNumberLiteral) {
    SwitchInput("1 2 3 \"ok\"\n");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_EQ(Node::kIntLiteral, ast->kind());
    ASSERT_TRUE(ast->IsIntLiteral());
    EXPECT_EQ(1, ast->AsIntLiteral()->value());
    
    ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    ASSERT_EQ(Node::kIntLiteral, ast->kind());
    ASSERT_TRUE(ast->IsIntLiteral());
    EXPECT_EQ(2, ast->AsIntLiteral()->value());
    
    ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    ASSERT_EQ(Node::kIntLiteral, ast->kind());
    ASSERT_TRUE(ast->IsIntLiteral());
    EXPECT_EQ(3, ast->AsIntLiteral()->value());
    
    ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    ASSERT_TRUE(ast->IsStringLiteral());
    EXPECT_STREQ("ok", ast->AsStringLiteral()->value()->data());
}

TEST_F(ParserTest, ParenOrLambdaLiteral) {
    SwitchInput("(1)\n");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsIntLiteral());
    EXPECT_EQ(1, ast->AsIntLiteral()->value());
    
    SwitchInput("()->1,2\n");
    ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsLambdaLiteral());
    auto lambda = ast->AsLambdaLiteral();
    EXPECT_TRUE(lambda->body()->IsList());
    EXPECT_EQ(0, lambda->prototype()->params_size());
    EXPECT_EQ(0, lambda->prototype()->return_types_size());
}

TEST_F(ParserTest, LambdaLiteral) {
    SwitchInput("(a:int, b:int)->a+b\n");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsLambdaLiteral());
    auto lambda = ast->AsLambdaLiteral();
    ASSERT_EQ(2, lambda->prototype()->params_size());
    ASSERT_FALSE(Type::Is(lambda->prototype()->param(0)));
    auto param = static_cast<VariableDeclaration::Item *>(lambda->prototype()->param(0));
    EXPECT_STREQ("a", param->identifier()->data());
    EXPECT_EQ(Type::kType_i32, param->type()->primary_type());
}

TEST_F(ParserTest, SimpleArithmetic) {
    SwitchInput("a + b - c / 100\n");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsSub());
    auto lhs = ast->AsSub()->lhs();
    ASSERT_TRUE(lhs->IsAdd());
    ASSERT_TRUE(lhs->AsAdd()->lhs()->IsIdentifier());
    ASSERT_TRUE(lhs->AsAdd()->rhs()->IsIdentifier());
    
    auto rhs = ast->AsSub()->rhs();
    ASSERT_TRUE(rhs->IsDiv());
    ASSERT_TRUE(rhs->AsDiv()->lhs()->IsIdentifier());
    ASSERT_TRUE(rhs->AsDiv()->rhs()->IsIntLiteral());
}

TEST_F(ParserTest, SimpleIfExpression) {
    SwitchInput("if (a > 0) -1 else 1");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsIfExpression());
    EXPECT_EQ(nullptr, ast->AsIfExpression()->initializer());
    EXPECT_TRUE(ast->AsIfExpression()->condition()->IsGreater());
    EXPECT_TRUE(ast->AsIfExpression()->then_clause()->IsIntLiteral());
    EXPECT_TRUE(ast->AsIfExpression()->else_clause()->IsIntLiteral());
}

TEST_F(ParserTest, IfExpressionWithInitializer) {
    SwitchInput("if (val a = foo(); a > 0) -1 else 1");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsIfExpression());
    auto expr = ast->AsIfExpression();
    EXPECT_TRUE(expr->initializer()->IsVariableDeclaration());
    EXPECT_TRUE(expr->condition()->IsGreater());
    EXPECT_TRUE(expr->then_clause()->IsIntLiteral());
    EXPECT_TRUE(expr->else_clause()->IsIntLiteral());
}

// a<b<c<int>>>
// a < b < c + 
TEST_F(ParserTest, IfExpressionWithElseIf) { // a<b<c<int>>() foo.Foo[~int,int]()
    SwitchInput("if (a > 0) -1 else if (a < 0) 1 else 0");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsIfExpression());
    auto expr = ast->AsIfExpression();
    EXPECT_TRUE(expr->else_clause()->IsIfExpression());
    expr = expr->else_clause()->AsIfExpression();
    EXPECT_TRUE(expr->condition()->IsLess());
    EXPECT_TRUE(expr->then_clause()->IsIntLiteral());
    EXPECT_EQ(1, expr->then_clause()->AsIntLiteral()->value());
    EXPECT_TRUE(expr->else_clause()->IsIntLiteral());
    EXPECT_EQ(0, expr->else_clause()->AsIntLiteral()->value());
}

TEST_F(ParserTest, WhenExpression) {
    SwitchInput("when {\n"
                "   a > 0 -> 0\n"
                "   a < 0 -> 1\n"
                "   a == 0 -> 2\n"
                "   else -> 3"
                "}\n");
    bool ok = true;
    auto ast = parser_.ParseWhenExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsWhenExpression());
    EXPECT_EQ(nullptr, ast->initializer());
    EXPECT_EQ(nullptr, ast->destination());
    ASSERT_EQ(3, ast->case_clauses_size());
    ASSERT_EQ(CaseWhenPattern::kExpectValue, ast->case_clause(0)->pattern());
    auto case_clause = WhenExpression::ExpectValueCase::Cast(ast->case_clause(0));
    ASSERT_TRUE(case_clause->match_value()->IsGreater());
    ASSERT_TRUE(case_clause->then_clause()->IsIntLiteral());
    EXPECT_EQ(0, case_clause->then_clause()->AsIntLiteral()->value());
    
    ASSERT_NE(nullptr, ast->else_clause());
    ASSERT_TRUE(ast->else_clause()->IsIntLiteral());
    EXPECT_EQ(3, ast->else_clause()->AsIntLiteral()->value());
}

TEST_F(ParserTest, WhenExpressionTypeCastingCases) {
    SwitchInput("when (val a = foo(); a) {\n"
                "   in 1 ..2 -> 0\n"
                "   v: int -> 1\n"
                "   v: f32 -> 2\n"
                "   else -> 3"
                "}\n");
    bool ok = true;
    auto ast = parser_.ParseWhenExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsWhenExpression());
    ASSERT_NE(nullptr, ast->AsWhenExpression()->initializer());
    ASSERT_TRUE(ast->AsWhenExpression()->initializer()->IsVariableDeclaration());
}

TEST_F(ParserTest, Instantiation) {
    SwitchInput("foo.Foo<int>()\n");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsCalling());
    ASSERT_TRUE(ast->AsCalling()->callee()->IsInstantiation());
    auto inst = ast->AsCalling()->callee()->AsInstantiation();
    ASSERT_TRUE(inst->primary()->IsDot());
    ASSERT_EQ(1, inst->generic_args_size());
    EXPECT_EQ(Type::kType_i32, inst->generic_arg(0)->primary_type());
}

TEST_F(ParserTest, InstantiationNested) {
    SwitchInput("foo.Foo<Bar<int, Baz<int, string> > >()\n");
    bool ok = true;
    auto ast = parser_.ParseExpression(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
}


TEST_F(ParserTest, Assigment) {
    SwitchInput("a = 1\n");
    bool ok = true;
    auto ast = parser_.ParseStatement(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsAssignment());
    auto stmt = ast->AsAssignment();
    ASSERT_EQ(1, stmt->lvals_size());
    ASSERT_TRUE(stmt->lval(0)->IsIdentifier());
    ASSERT_EQ(1, stmt->rvals_size());
    ASSERT_TRUE(stmt->rval(0)->IsIntLiteral());
}

TEST_F(ParserTest, AssigmentMultiLVal) {
    SwitchInput("a, b, c = 1\n");
    bool ok = true;
    auto ast = parser_.ParseStatement(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsAssignment());
    auto stmt = ast->AsAssignment();
    ASSERT_EQ(3, stmt->lvals_size());
    ASSERT_TRUE(stmt->lval(0)->IsIdentifier());
    ASSERT_TRUE(stmt->lval(1)->IsIdentifier());
    ASSERT_TRUE(stmt->lval(2)->IsIdentifier());
    ASSERT_EQ(1, stmt->rvals_size());
    ASSERT_TRUE(stmt->rval(0)->IsIntLiteral());
}

TEST_F(ParserTest, AssigmentMultiRVal) {
    SwitchInput("a, b, c = 1, 2, 3\n");
    bool ok = true;
    auto ast = parser_.ParseStatement(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    ASSERT_TRUE(ast->IsAssignment());
    auto stmt = ast->AsAssignment();
    ASSERT_EQ(3, stmt->lvals_size());
    ASSERT_TRUE(stmt->lval(0)->IsIdentifier());
    ASSERT_TRUE(stmt->lval(1)->IsIdentifier());
    ASSERT_TRUE(stmt->lval(2)->IsIdentifier());
    ASSERT_EQ(3, stmt->rvals_size());
    ASSERT_TRUE(stmt->rval(0)->IsIntLiteral());
    ASSERT_TRUE(stmt->rval(1)->IsIntLiteral());
    ASSERT_TRUE(stmt->rval(2)->IsIntLiteral());
}

TEST_F(ParserTest, FunctionDeclaration) {
    SwitchInput("native fun foo()\n"
                "fun bar(@Foo a: int)->1\n");
    bool ok = true;
    auto ast = parser_.ParseFunctionDeclaration(&ok);
    ASSERT_TRUE(ok);
    ASSERT_NE(nullptr, ast);
    
    EXPECT_TRUE(ast->IsNative());
    EXPECT_STREQ("foo", ast->name()->data());
    EXPECT_EQ(0, ast->generic_params_size());
    EXPECT_EQ(0, ast->prototype()->params_size());
    EXPECT_EQ(0, ast->prototype()->return_types_size());
    
    ast = parser_.ParseFunctionDeclaration(&ok);
    ASSERT_TRUE(ok);
    EXPECT_TRUE(ast->IsDefault());
    EXPECT_STREQ("bar", ast->name()->data());
    
    auto param = static_cast<VariableDeclaration::Item *>(ast->prototype()->param(0));
    EXPECT_STREQ("a", param->identifier()->data());
    ASSERT_NE(nullptr, param->annotations());
    ASSERT_EQ(1, param->annotations()->annotations_size());
    auto anno = param->annotations()->annotation(0);
    EXPECT_STREQ("Foo", anno->name()->name()->data());
    
}

} // namespace cpl

} // namespace yalx
