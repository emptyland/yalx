#include "compiler/ast.h"

namespace yalx {

namespace cpl {

namespace {

class AstPrinter : public AstVisitor {
public:
    AstPrinter(int indent): indent_(indent) {}

    // file_unit:
    //   package: ''
    //   imports:
    //     ... as ...
    //   statements:
    //     function_declaration: (a,b,c)->a,b,c | name: aaxx
    //       name: foo
    //       prototype: (a,b,c): a,b,c
    //       vargs: false
    //       body:
    //         return:
    //           - a
    //           - b
    //
    //
    int VisitPackage(Package *node) override { return -1; }
    int VisitFileUnit(FileUnit *node) override { return -1; }
    int VisitBlock(Block *node) override { return -1; }
    int VisitList(List *node) override { return -1; }
    int VisitAssignment(Assignment *node) override { return -1; }
    int VisitStructDefinition(StructDefinition *node) override { return -1; }
    int VisitClassDefinition(ClassDefinition *node) override { return -1; }
    int VisitEnumDefinition(EnumDefinition *node) override { return -1; }
    int VisitAnnotationDefinition(AnnotationDefinition *node) override { return -1; }
    int VisitInterfaceDefinition(InterfaceDefinition *node) override { return -1; }
    int VisitFunctionDeclaration(FunctionDeclaration *node) override { return -1; }
    int VisitAnnotationDeclaration(AnnotationDeclaration *node) override { return -1; }
    int VisitAnnotation(Annotation *node) override { return -1; }
    int VisitBreak(Break *node) override { return -1; }
    int VisitContinue(Continue *node) override { return -1; }
    int VisitReturn(Return *node) override { return -1; }
    int VisitThrow(Throw *node) override { return -1; }
    int VisitRunCoroutine(RunCoroutine *node) override { return -1; }
    int VisitWhileLoop(WhileLoop *ast) override { return -1; }
    int VisitUnlessLoop(UnlessLoop *node) override { return -1; }
    int VisitForeachLoop(ForeachLoop *node) override { return -1; }
    int VisitStringTemplate(StringTemplate *node) override { return -1; }
    int VisitInstantiation(Instantiation *node) override { return -1; }
    int VisitOr(Or *node) override { return -1; }
    int VisitAdd(Add *node) override { return -1; }
    int VisitAnd(And *node) override { return -1; }
    int VisitDiv(Div *node) override { return -1; }
    int VisitDot(Dot *node) override { return -1; }
    int VisitMod(Mod *node) override { return -1; }
    int VisitMul(Mul *node) override { return -1; }
    int VisitNot(Not *node) override { return -1; }
    int VisitSub(Sub *node) override { return -1; }
    int VisitLess(Less *node) override { return -1; }
    int VisitRecv(Recv *node) override { return -1; }
    int VisitSend(Send *node) override { return -1; }
    int VisitEqual(Equal *node) override { return -1; }
    int VisitCalling(Calling *node) override { return -1; }
    int VisitCasting(Casting *node) override { return -1; }
    int VisitGreater(Greater *node) override { return -1; }
    int VisitTesting(Testing *node) override { return -1; }
    int VisitNegative(Negative *node) override { return -1; }
    int VisitAssertedGet(AssertedGet *node) override { return -1; }
    int VisitIdentifier(Identifier *node) override { return -1; }
    int VisitNotEqual(NotEqual *node) override { return -1; }
    int VisitBitwiseOr(BitwiseOr *node) override { return -1; }
    int VisitLessEqual(LessEqual *node) override { return -1; }
    int VisitBitwiseAnd(BitwiseAnd *node) override { return -1; }
    int VisitBitwiseShl(BitwiseShl *node) override { return -1; }
    int VisitBitwiseShr(BitwiseShr *node) override { return -1; }
    int VisitBitwiseXor(BitwiseXor *node) override { return -1; }
    int VisitF32Literal(F32Literal *node) override { return -1; }
    int VisitF64Literal(F64Literal *node) override { return -1; }
    int VisitI64Literal(I64Literal *node) override { return -1; }
    int VisitIndexedGet(IndexedGet *node) override { return -1; }
    int VisitI8Literal(I8Literal *node) override { return -1; }
    int VisitU8Literal(U8Literal *node) override { return -1; }
    int VisitI16Literal(I16Literal *node) override { return -1; }
    int VisitU16Literal(U16Literal *node) override { return -1; }
    int VisitI32Literal(I32Literal *node) override { return -1; }
    int VisitU32Literal(U32Literal *node) override { return -1; }
    int VisitIntLiteral(IntLiteral *node) override { return -1; }
    int VisitU64Literal(U64Literal *node) override { return -1; }
    int VisitBoolLiteral(BoolLiteral *node) override { return -1; }
    int VisitUnitLiteral(UnitLiteral *node) override { return -1; }
    int VisitEmptyLiteral(EmptyLiteral *node) override { return -1; }
    int VisitGreaterEqual(GreaterEqual *node) override { return -1; }
    int VisitIfExpression(IfExpression *node) override { return -1; }
    int VisitLambdaLiteral(LambdaLiteral *node) override { return -1; }
    int VisitStringLiteral(StringLiteral *node) override { return -1; }
    int VisitWhenExpression(WhenExpression *node) override { return -1; }
    int VisitBitwiseNegative(BitwiseNegative *node) override { return -1; }
    int VisitArrayInitializer(ArrayInitializer *node) override { return -1; }
    int VisitObjectDeclaration(ObjectDeclaration *node) override { return -1; }
    int VisitVariableDeclaration(VariableDeclaration *node) override { return -1; }
    int VisitUIntLiteral(UIntLiteral *node) override { return -1; }
    int VisitTryCatchExpression(TryCatchExpression *node) override { return -1; }
    int VisitOptionLiteral(OptionLiteral *node) override { return -1; }
    int VisitCharLiteral(CharLiteral *node) override { return -1; }
    int VisitChannelInitializer(ChannelInitializer *node) override { return -1; }
    //void Visit
    std::string buf_;
private:
    const int indent_;
    int depth_ = 0;
}; // class AstPrinter

} // namespace

void PrintAst(AstNode *ast, int indent, std::string *buf) {
    AstPrinter printer(indent);
    ast->Accept(&printer);
    *buf = std::move(printer.buf_);
}

} // namespace cpl

} // namespace yalx
