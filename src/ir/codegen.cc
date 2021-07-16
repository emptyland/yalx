#include "ir/codegen.h"
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

class IRCodeEnvScope;
class IRCodePkgScope;
class IRCodeFileScope;
class IRCodeStructureScope;
class IRCodeFunScope;
class IRCodeBlockScope;

struct Symbol {
    enum Kind {
        kNotFound,
        kUnknown,
        kModel,
        kVal,
        kFun,
        kField,
    };
    Kind kind;
    cpl::Statement *node;
    union {
        Model *model;
        Value *value;
        Function *fun;
        ptrdiff_t field_offset; // offset of field
    } core;
    IRCodeEnvScope *owns;
};

class IRCodeEnvScope {
public:
    enum Block {
        kPlainBlock,
        kLoopBlock,
        kBranchBlock,
    };
    
    virtual ~IRCodeEnvScope();
    
    virtual IRCodePkgScope *NearlyPkgScope() { return !top_ ? nullptr : top_->NearlyPkgScope(); };
    virtual IRCodeFileScope *NearlyFileScope() { return !top_ ? nullptr : top_->NearlyFileScope(); };
    virtual IRCodeStructureScope *NearlyStructureScope() { return !top_ ? nullptr : top_->NearlyStructureScope(); };
    virtual IRCodeFunScope *NearlyFunScope() { return !top_ ? nullptr : top_->NearlyFunScope(); };
    virtual IRCodeBlockScope *NearlyBlockScope() { return !top_ ? nullptr : top_->NearlyBlockScope(); };
    
    virtual Symbol FindLocalSymbol(std::string_view name) = 0;
    virtual Symbol FindReachableSymbol(std::string_view name) = 0;
protected:
    IRCodeEnvScope(IRCodeEnvScope **location);
    
    IRCodeEnvScope **location_;
    IRCodeEnvScope *top_ = nullptr;
}; // class IRCodeEnvScope

class IRCodePkgScope : public IRCodeEnvScope {
public:
    IRCodePkgScope *NearlyPkgScope() override { return this; }
    
private:
    struct Slot {
        cpl::FileUnit *file_unit;
        cpl::Statement *node;
        Model *model;
        Value *value;
        Function *fun;
    };
    
    //std::map<std::string_view, Slot> symbols_;
    std::vector<IRCodeFileScope *> file_scopes_;
}; // class IRCodePkgScope

class IRCodeFileScope : public IRCodeEnvScope {
public:
    IRCodeFileScope *NearlyFileScope() override { return this; };
    
private:
    cpl::FileUnit *file_unit_;
}; // class IRCodeFileScope

class IRCodeStructureScope : public IRCodeEnvScope {
public:
    IRCodeStructureScope *NearlyStructureScope() override { return this; }
}; // class IRCodeStructureScope

class IRCodeFunScope : public IRCodeEnvScope {
public:
    IRCodeFunScope *NearlyFunScope() override { return this; }
}; // class IRCodeStructureScope

// TODO add branch scope
class IRCodeBlockScope : public IRCodeEnvScope {
public:
    IRCodeBlockScope *NearlyBlockScope() override { return this; }
    
private:
    cpl::Statement *ast_;
    BasicBlock *block_;
}; // class IRCodeBlockScope


class IRGeneratorAstVisitor : public cpl::AstVisitor {
public:
    IRGeneratorAstVisitor(IntermediateRepresentationGenerator *owns)
        : owns_(owns)
        , ops_(owns->arena_) {
    }
    
    // Module(base::Arena *arena, const String *name, const String *path, const String *full_path)
    void VisitPackage(cpl::Package *node) override {
        assert(module_ == nullptr);
        
        module_ = new (arena()) Module(arena(), node->name(), node->path(), node->full_path());
        PrepareAllSymbols(node);
        if (fail()) {
            return;
        }
        
        for (auto file_unit : node->source_files()) {
            file_unit->Accept(this);
            if (fail()) {
                return;
            }
        }
    }

    void VisitFileUnit(cpl::FileUnit *node) override { UNREACHABLE(); }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    void VisitBlock(cpl::Block *node) override { UNREACHABLE(); }
    void VisitList(cpl::List *node) override { UNREACHABLE(); }
    void VisitAssignment(cpl::Assignment *node) override { UNREACHABLE(); }
    void VisitStructDefinition(cpl::StructDefinition *node) override { UNREACHABLE(); }
    void VisitClassDefinition(cpl::ClassDefinition *node) override { UNREACHABLE(); }
    void VisitAnnotationDefinition(cpl::AnnotationDefinition *node) override { UNREACHABLE(); }
    void VisitInterfaceDefinition(cpl::InterfaceDefinition *node) override { UNREACHABLE(); }
    void VisitFunctionDeclaration(cpl::FunctionDeclaration *node) override { UNREACHABLE(); }
    void VisitAnnotationDeclaration(cpl::AnnotationDeclaration *node) override { UNREACHABLE(); }
    void VisitAnnotation(cpl::Annotation *node) override { UNREACHABLE(); }
    void VisitBreak(cpl::Break *node) override { UNREACHABLE(); }
    void VisitContinue(cpl::Continue *node) override { UNREACHABLE(); }
    void VisitReturn(cpl::Return *node) override { UNREACHABLE(); }
    void VisitThrow(cpl::Throw *node) override { UNREACHABLE(); }
    void VisitRunCoroutine(cpl::RunCoroutine *node) override { UNREACHABLE(); }
    void VisitWhileLoop(cpl::WhileLoop *node) override { UNREACHABLE(); }
    void VisitUnlessLoop(cpl::UnlessLoop *node) override { UNREACHABLE(); }
    void VisitForeachLoop(cpl::ForeachLoop *node) override { UNREACHABLE(); }
    void VisitStringTemplate(cpl::StringTemplate *node) override { UNREACHABLE(); }
    void VisitInstantiation(cpl::Instantiation *node) override { UNREACHABLE(); }
    void VisitOr(cpl::Or *node) override { UNREACHABLE(); }
    void VisitAdd(cpl::Add *node) override { UNREACHABLE(); }
    void VisitAnd(cpl::And *node) override { UNREACHABLE(); }
    void VisitDiv(cpl::Div *node) override { UNREACHABLE(); }
    void VisitDot(cpl::Dot *node) override { UNREACHABLE(); }
    void VisitMod(cpl::Mod *node) override { UNREACHABLE(); }
    void VisitMul(cpl::Mul *node) override { UNREACHABLE(); }
    void VisitNot(cpl::Not *node) override { UNREACHABLE(); }
    void VisitSub(cpl::Sub *node) override { UNREACHABLE(); }
    void VisitLess(cpl::Less *node) override { UNREACHABLE(); }
    void VisitRecv(cpl::Recv *node) override { UNREACHABLE(); }
    void VisitSend(cpl::Send *node) override { UNREACHABLE(); }
    void VisitEqual(cpl::Equal *node) override { UNREACHABLE(); }
    void VisitCalling(cpl::Calling *node) override { UNREACHABLE(); }
    void VisitCasting(cpl::Casting *node) override { UNREACHABLE(); }
    void VisitGreater(cpl::Greater *node) override { UNREACHABLE(); }
    void VisitTesting(cpl::Testing *node) override { UNREACHABLE(); }
    void VisitNegative(cpl::Negative *node) override { UNREACHABLE(); }
    void VisitIdentifier(cpl::Identifier *node) override { UNREACHABLE(); }
    void VisitNotEqual(cpl::NotEqual *node) override { UNREACHABLE(); }
    void VisitBitwiseOr(cpl::BitwiseOr *node) override { UNREACHABLE(); }
    void VisitLessEqual(cpl::LessEqual *node) override { UNREACHABLE(); }
    void VisitBitwiseAnd(cpl::BitwiseAnd *node) override { UNREACHABLE(); }
    void VisitBitwiseShl(cpl::BitwiseShl *node) override { UNREACHABLE(); }
    void VisitBitwiseShr(cpl::BitwiseShr *node) override { UNREACHABLE(); }
    void VisitBitwiseXor(cpl::BitwiseXor *node) override { UNREACHABLE(); }
    void VisitF32Literal(cpl::F32Literal *node) override { UNREACHABLE(); }
    void VisitF64Literal(cpl::F64Literal *node) override { UNREACHABLE(); }
    void VisitI64Literal(cpl::I64Literal *node) override { UNREACHABLE(); }
    void VisitIndexedGet(cpl::IndexedGet *node) override { UNREACHABLE(); }
    void VisitIntLiteral(cpl::IntLiteral *node) override { UNREACHABLE(); }
    void VisitU64Literal(cpl::U64Literal *node) override { UNREACHABLE(); }
    void VisitBoolLiteral(cpl::BoolLiteral *node) override { UNREACHABLE(); }
    void VisitUnitLiteral(cpl::UnitLiteral *node) override { UNREACHABLE(); }
    void VisitEmptyLiteral(cpl::EmptyLiteral *node) override { UNREACHABLE(); }
    void VisitGreaterEqual(cpl::GreaterEqual *node) override { UNREACHABLE(); }
    void VisitIfExpression(cpl::IfExpression *node) override { UNREACHABLE(); }
    void VisitLambdaLiteral(cpl::LambdaLiteral *node) override { UNREACHABLE(); }
    void VisitStringLiteral(cpl::StringLiteral *node) override { UNREACHABLE(); }
    void VisitWhenExpression(cpl::WhenExpression *node) override { UNREACHABLE(); }
    void VisitBitwiseNegative(cpl::BitwiseNegative *node) override { UNREACHABLE(); }
    void VisitArrayInitializer(cpl::ArrayInitializer *node) override { UNREACHABLE(); }
    void VisitObjectDeclaration(cpl::ObjectDeclaration *node) override { UNREACHABLE(); }
    void VisitVariableDeclaration(cpl::VariableDeclaration *node) override { UNREACHABLE(); }
    void VisitUIntLiteral(cpl::UIntLiteral *node) override { UNREACHABLE(); }
    void VisitTryCatchExpression(cpl::TryCatchExpression *node) override { UNREACHABLE(); }
    
    
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
