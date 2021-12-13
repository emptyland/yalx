#include "ir/codegen.h"
#include "ir/scope.h"
#include "ir/operators-factory.h"
#include "ir/operator.h"
#include "ir/node.h"
#include "ir/metadata.h"
#include "compiler/ast.h"
#include "compiler/syntax-feedback.h"
#include "base/checking.h"
#include "base/format.h"

namespace yalx {

namespace ir {




class IRGeneratorAstVisitor : public cpl::AstVisitor {
public:
    IRGeneratorAstVisitor(IntermediateRepresentationGenerator *owns)
        : owns_(owns)
        , ops_(owns->arena_) {
    }
    
    // Module(base::Arena *arena, const String *name, const String *path, const String *full_path)
    int VisitPackage(cpl::Package *node) override {
        assert(module_ == nullptr);
        
        module_ = new (arena()) Module(arena(), node->name(), node->path(), node->full_path());
        PrepareAllSymbols(node);
        if (fail()) {
            return -1;
        }
        
        for (auto file_unit : node->source_files()) {
            file_unit->Accept(this);
            if (fail()) {
                return -1;
            }
        }
        return 0;
    }

    int VisitFileUnit(cpl::FileUnit *node) override { UNREACHABLE(); }
    int VisitBlock(cpl::Block *node) override { UNREACHABLE(); }
    int VisitList(cpl::List *node) override { UNREACHABLE(); }
    int VisitAssignment(cpl::Assignment *node) override { UNREACHABLE(); }
    int VisitStructDefinition(cpl::StructDefinition *node) override { UNREACHABLE(); }
    int VisitClassDefinition(cpl::ClassDefinition *node) override { UNREACHABLE(); }
    int VisitAnnotationDefinition(cpl::AnnotationDefinition *node) override { UNREACHABLE(); }
    int VisitInterfaceDefinition(cpl::InterfaceDefinition *node) override { UNREACHABLE(); }
    int VisitFunctionDeclaration(cpl::FunctionDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotationDeclaration(cpl::AnnotationDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotation(cpl::Annotation *node) override { UNREACHABLE(); }
    int VisitBreak(cpl::Break *node) override { UNREACHABLE(); }
    int VisitContinue(cpl::Continue *node) override { UNREACHABLE(); }
    int VisitReturn(cpl::Return *node) override { UNREACHABLE(); }
    int VisitThrow(cpl::Throw *node) override { UNREACHABLE(); }
    int VisitRunCoroutine(cpl::RunCoroutine *node) override { UNREACHABLE(); }
    int VisitWhileLoop(cpl::WhileLoop *node) override { UNREACHABLE(); }
    int VisitUnlessLoop(cpl::UnlessLoop *node) override { UNREACHABLE(); }
    int VisitForeachLoop(cpl::ForeachLoop *node) override { UNREACHABLE(); }
    int VisitStringTemplate(cpl::StringTemplate *node) override { UNREACHABLE(); }
    int VisitInstantiation(cpl::Instantiation *node) override { UNREACHABLE(); }
    int VisitOr(cpl::Or *node) override { UNREACHABLE(); }
    int VisitAdd(cpl::Add *node) override { UNREACHABLE(); }
    int VisitAnd(cpl::And *node) override { UNREACHABLE(); }
    int VisitDiv(cpl::Div *node) override { UNREACHABLE(); }
    int VisitDot(cpl::Dot *node) override { UNREACHABLE(); }
    int VisitMod(cpl::Mod *node) override { UNREACHABLE(); }
    int VisitMul(cpl::Mul *node) override { UNREACHABLE(); }
    int VisitNot(cpl::Not *node) override { UNREACHABLE(); }
    int VisitSub(cpl::Sub *node) override { UNREACHABLE(); }
    int VisitLess(cpl::Less *node) override { UNREACHABLE(); }
    int VisitRecv(cpl::Recv *node) override { UNREACHABLE(); }
    int VisitSend(cpl::Send *node) override { UNREACHABLE(); }
    int VisitEqual(cpl::Equal *node) override { UNREACHABLE(); }
    int VisitCalling(cpl::Calling *node) override { UNREACHABLE(); }
    int VisitCasting(cpl::Casting *node) override { UNREACHABLE(); }
    int VisitGreater(cpl::Greater *node) override { UNREACHABLE(); }
    int VisitTesting(cpl::Testing *node) override { UNREACHABLE(); }
    int VisitNegative(cpl::Negative *node) override { UNREACHABLE(); }
    int VisitIdentifier(cpl::Identifier *node) override { UNREACHABLE(); }
    int VisitNotEqual(cpl::NotEqual *node) override { UNREACHABLE(); }
    int VisitBitwiseOr(cpl::BitwiseOr *node) override { UNREACHABLE(); }
    int VisitLessEqual(cpl::LessEqual *node) override { UNREACHABLE(); }
    int VisitBitwiseAnd(cpl::BitwiseAnd *node) override { UNREACHABLE(); }
    int VisitBitwiseShl(cpl::BitwiseShl *node) override { UNREACHABLE(); }
    int VisitBitwiseShr(cpl::BitwiseShr *node) override { UNREACHABLE(); }
    int VisitBitwiseXor(cpl::BitwiseXor *node) override { UNREACHABLE(); }
    int VisitF32Literal(cpl::F32Literal *node) override { UNREACHABLE(); }
    int VisitF64Literal(cpl::F64Literal *node) override { UNREACHABLE(); }
    int VisitI64Literal(cpl::I64Literal *node) override { UNREACHABLE(); }
    int VisitIndexedGet(cpl::IndexedGet *node) override { UNREACHABLE(); }
    int VisitIntLiteral(cpl::IntLiteral *node) override { UNREACHABLE(); }
    int VisitU64Literal(cpl::U64Literal *node) override { UNREACHABLE(); }
    int VisitBoolLiteral(cpl::BoolLiteral *node) override { UNREACHABLE(); }
    int VisitUnitLiteral(cpl::UnitLiteral *node) override { UNREACHABLE(); }
    int VisitEmptyLiteral(cpl::EmptyLiteral *node) override { UNREACHABLE(); }
    int VisitGreaterEqual(cpl::GreaterEqual *node) override { UNREACHABLE(); }
    int VisitIfExpression(cpl::IfExpression *node) override { UNREACHABLE(); }
    int VisitLambdaLiteral(cpl::LambdaLiteral *node) override { UNREACHABLE(); }
    int VisitStringLiteral(cpl::StringLiteral *node) override { UNREACHABLE(); }
    int VisitWhenExpression(cpl::WhenExpression *node) override { UNREACHABLE(); }
    int VisitBitwiseNegative(cpl::BitwiseNegative *node) override { UNREACHABLE(); }
    int VisitArrayInitializer(cpl::ArrayInitializer *node) override { UNREACHABLE(); }
    int VisitObjectDeclaration(cpl::ObjectDeclaration *node) override { UNREACHABLE(); }
    int VisitVariableDeclaration(cpl::VariableDeclaration *node) override { UNREACHABLE(); }
    int VisitUIntLiteral(cpl::UIntLiteral *node) override { UNREACHABLE(); }
    int VisitTryCatchExpression(cpl::TryCatchExpression *node) override { UNREACHABLE(); }
    int VisitOptionLiteral(cpl::OptionLiteral *node) override { UNREACHABLE(); }
    int VisitAssertedGet(cpl::AssertedGet *node) override { UNREACHABLE(); }
    
    friend class IRCodeEnvScope;
    friend class IRCodePkgScope;
    friend class IRCodeFileScope;
    friend class IRCodeStructureScope;
    friend class IRCodeFunScope;
    friend class IRCodeBlockScope;
private:
    struct Symbol {
        cpl::FileUnit *file_unit;
        cpl::AstNode  *node;
    };
    
    void PrepareAllSymbols(cpl::Package *pkg) {
        for (auto file_unit : pkg->source_files()) {
            feedback()->set_file_name(file_unit->file_name()->ToString());
            
            
            for (auto stmt : file_unit->statements()) {
                SourcePositionTable::Scope scope(file_unit->file_name(), stmt->source_position(),
                                                 module_->mutable_source_position_table());
                if (!CheckDuplicatedSymbols(file_unit, stmt)) {
                    return;
                }

                switch (stmt->kind()) {
                    case cpl::Node::kVariableDeclaration: {
                        auto decl = stmt->AsVariableDeclaration();
                        for (auto item : decl->variables()) {
                            auto val = Value::New0(arena(), scope.Position(), Types::Void, ops()->GlobalValue());
                            module_->InsertGlobalValue(item->identifier(), val);
                        }
                    } break;

                    case cpl::Node::kClassDefinition: {
                        auto def = stmt->AsClassDefinition();
                        module_->NewClassModel(def->name(), nullptr);
                    } break;

                    case cpl::Node::kStructDefinition: {
                        auto def = stmt->AsStructDefinition();
                        module_->NewStructModel(def->name(), nullptr);
                    } break;

                    case cpl::Node::kObjectDeclaration: {
                        auto decl = stmt->AsObjectDeclaration();
                        auto class_name = base::Sprintf("%s$class", decl->name()->data());
                        auto model = module_->NewClassModel(String::New(arena(), class_name), nullptr);
                        
                        auto val = Value::New0(arena(), scope.Position(), Type::Ref(model), ops()->GlobalValue());
                        module_->InsertGlobalValue(decl->name(), val);
                    } break;

                    case cpl::Node::kFunctionDeclaration: {
                        auto decl = stmt->AsFunctionDeclaration();
                        auto prototype = new (arena()) PrototypeModel(arena(), decl->prototype()->vargs());
                        module_->NewFunction(decl->name(), prototype);
                    } break;

                    case cpl::Node::kAnnotationDefinition:
                        // Ignore
                    default:
                        break;
                }
            }
        }
    }
    
    bool CheckDuplicatedSymbols(cpl::FileUnit *file_unit, cpl::Statement *stmt) {
        if (cpl::Declaration::Is(stmt)) {
            auto decl = static_cast<cpl::Declaration *>(stmt);
            for (int i = 0; i < decl->ItemSize(); i++) {
                auto name = decl->AtItem(i)->Identifier();
                if (InnerSymbolExists(name)) {
                    status_ = ERR_CORRUPTION("Duplicated symbol name");
                    feedback()->Printf(stmt->source_position(), "Duplicated symbol name: \"%s\"", name->data());
                    return false;
                }
            }
        } else {
            assert(cpl::Definition::Is(stmt));
            auto name = static_cast<cpl::Definition *>(stmt)->name();
            if (InnerSymbolExists(name)) {
                status_ = ERR_CORRUPTION("Duplicated symbol name");
                feedback()->Printf(stmt->source_position(), "Duplicated symbol name: \"%s\"", name->data());
                return false;
            }
        }
        
        return true;
    }
    
    bool InnerSymbolExists(const String *name) {
        return module_->FindFunOrNull(name->ToSlice()) == nullptr && module_->FindValOrNull(name->ToSlice()) == nullptr;
    }
    
    bool ok() { return status_.ok(); }
    bool fail() { return status_.fail(); }
    base::Arena *arena() { return owns_->arena_; }
    cpl::SyntaxFeedback *feedback() { return owns_->error_feedback_; }
    OperatorsFactory *ops() { return &ops_; }
    
    IntermediateRepresentationGenerator *const owns_;
    IRCodeEnvScope *top_ = nullptr;
    Module *module_ = nullptr;
    std::map<std::string_view, Symbol> inner_symbols_;
    base::Status status_ = base::Status::OK();
    OperatorsFactory ops_;
}; // class IntermediateRepresentationGenerator::AstVisitor


IntermediateRepresentationGenerator::IntermediateRepresentationGenerator(base::Arena *arena,
                                                                         cpl::Package *entry,
                                                                         cpl::SyntaxFeedback *error_feedback)
    : arena_(DCHECK_NOTNULL(arena))
    , entry_(entry)
    , error_feedback_(error_feedback)
    , modules_(arena) {
}

base::Status IntermediateRepresentationGenerator::Run() {
    if (auto rs = RecursiveGeneratePackage(entry_); rs.fail()) {
        return rs;
    }
    IRGeneratorAstVisitor visitor(this);
    entry_->Accept(&visitor);
    return base::Status::OK();
}

base::Status IntermediateRepresentationGenerator::RecursiveGeneratePackage(cpl::Package *root) {
    if (root->IsTerminator()) {
        return base::Status::OK();
    }
    
    for (auto pkg : root->dependences()) {
        if (auto rs = RecursiveGeneratePackage(pkg); rs.fail()) {
            return rs;
        }
        IRGeneratorAstVisitor visitor(this);
        pkg->Accept(&visitor);
    }
    return base::Status::OK();
}

} // namespace ir

} // namespace yalx
