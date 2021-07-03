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
    void VisitFileUnit(FileUnit *node) override {
        
    }
    
    void VisitBlock(Block *node) override {
        
    }
    
    void VisitList(List *node) override {
        
    }
    
    void VisitAssignment(Assignment *node) override {
        
    }
    
    void VisitStructDefinition(StructDefinition *node) override {
        
    }
    
    void VisitClassDefinition(ClassDefinition *node) override {
        
    }
    
    void VisitAnnotationDefinition(AnnotationDefinition *node) override {
        
    }
    
    void VisitInterfaceDefinition(InterfaceDefinition *node) override {
        
    }
    
    void VisitFunctionDeclaration(FunctionDeclaration *node) override {
        
    }
    
    void VisitAnnotationDeclaration(AnnotationDeclaration *node) override {
        
    }
    
    void VisitAnnotation(Annotation *node) override {
        
    }
    
    void VisitBreak(Break *node) override {
        
    }
    
    void VisitContinue(Continue *node) override {
        
    }
    
    void VisitReturn(Return *node) override {
        
    }
    
    void VisitThrow(Throw *node) override {}
    void VisitRunCoroutine(RunCoroutine *node) override {}
    void VisitWhileLoop(WhileLoop *ast) override {}
    void VisitUnlessLoop(UnlessLoop *node) override {}
    void VisitForeachLoop(ForeachLoop *node) override {}
    
    void VisitInstantiation(Instantiation *node) override {
        
    }
    
    void VisitOr(Or *node) override {}
    void VisitAdd(Add *node) override {}
    void VisitAnd(And *node) override {}
    void VisitDiv(Div *node) override {}
    void VisitDot(Dot *node) override {}
    void VisitMod(Mod *node) override {}
    void VisitMul(Mul *node) override {}
    void VisitNot(Not *node) override {}
    void VisitSub(Sub *node) override {}
    void VisitLess(Less *node) override {}
    void VisitRecv(Recv *node) override {}
    void VisitSend(Send *node) override {}
    void VisitEqual(Equal *node) override {}
    void VisitCalling(Calling *node) override {}
    void VisitCasting(Casting *node) override {}
    void VisitGreater(Greater *node) override {}
    void VisitTesting(Testing *node) override {}
    void VisitNegative(Negative *node) override {}
    void VisitIdentifier(Identifier *node) override {}
    void VisitNotEqual(NotEqual *node) override {}
    void VisitBitwiseOr(BitwiseOr *node) override {}
    void VisitLessEqual(LessEqual *node) override {}
    void VisitBitwiseAnd(BitwiseAnd *node) override {}
    void VisitBitwiseShl(BitwiseShl *node) override {}
    void VisitBitwiseShr(BitwiseShr *node) override {}
    void VisitBitwiseXor(BitwiseXor *node) override {}
    void VisitF32Literal(F32Literal *node) override {}
    void VisitF64Literal(F64Literal *node) override {}
    void VisitI64Literal(I64Literal *node) override {}
    void VisitIndexedGet(IndexedGet *node) override {}
    void VisitIntLiteral(IntLiteral *node) override {}
    void VisitU64Literal(U64Literal *node) override {}
    void VisitBoolLiteral(BoolLiteral *node) override {}
    void VisitUnitLiteral(UnitLiteral *node) override {}
    void VisitEmptyLiteral(EmptyLiteral *node) override {}
    void VisitGreaterEqual(GreaterEqual *node) override {}
    void VisitIfExpression(IfExpression *node) override {}
    void VisitLambdaLiteral(LambdaLiteral *node) override {}
    void VisitStringLiteral(StringLiteral *node) override {}
    void VisitWhenExpression(WhenExpression *node) override {}
    void VisitBitwiseNegative(BitwiseNegative *node) override {}
    void VisitArrayInitializer(ArrayInitializer *node) override {}
    void VisitObjectDeclaration(ObjectDeclaration *node) override {}
    void VisitVariableDeclaration(VariableDeclaration *node) override {}
    void VisitUIntLiteral(UIntLiteral *node) override {}
    void VisitTryCatchExpression(TryCatchExpression *node) override {}

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
