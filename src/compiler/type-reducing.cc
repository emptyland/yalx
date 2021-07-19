#include "compiler/type-reducing.h"
#include "compiler/syntax-feedback.h"
#include "compiler/scope.h"
#include "compiler/ast.h"
#include <stack>

namespace yalx {

namespace cpl {

class TypeReducingVisitor : public AstVisitor {
public:
    TypeReducingVisitor(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback);
    
    base::Status Reduce() {
        if (auto rs = RecursiveReducePackage(entry_); rs.fail()) {
            return rs;
        }
        entry_->Accept(this);
        return status_;
    }
    
    base::Status RecursiveReducePackage(Package *root) {
        if (root->IsTerminator()) {
            return base::Status::OK();
        }
        for (auto pkg : root->dependences()) {
            if (auto rs = RecursiveReducePackage(pkg); rs.fail()) {
                return rs;
            }
            pkg->Accept(this);
            if (status_.fail()) {
                return status_;
            }
        }
        return base::Status::OK();
    }
    
private:
    int VisitPackage(Package *node) override {
        PackageScope scope(&location_, node);
        for (auto file_scope : scope.files()) {
            file_scope->Enter();
            error_feedback_->set_file_name(file_scope->file_unit()->file_name()->ToString());
            error_feedback_->set_package_name(node->name()->ToString());
            if (Reduce(file_scope->file_unit()) < 0) {
                file_scope->Exit();
                return -1;
            }
            file_scope->Exit();
        }
        return 0;
    }

    int VisitFileUnit(FileUnit *node) override {
        for (auto ast : node->statements()) {
            if (auto type = Reduce(ast); !type) {
                return -1;
            }
        }
        return Return(Unit());
    }

    int VisitBlock(Block *node) override {
        std::vector<Type *> types = { Unit() };
        for (auto ast : node->statements()) {
            types.clear();
            if (Reduce(ast, &types) < 0) {
                return -1;
            }
        }
        return Return(types);
    }
    
    int VisitList(List *node) override {
        std::vector<Type *> types;
        for (auto ast : node->expressions()) {
            if (Reduce(ast, &types) < 0) {
                return -1;
            }
        }
        assert(!types.empty());
        return Return(types);
    }
    
    int VisitVariableDeclaration(VariableDeclaration *node) override {
        for (auto var : node->variables()) {
            auto duplicated = location_->FindOrInsertSymbol(var->identifier()->ToSlice(), var);
            if (duplicated) {
                Feedback()->Printf(var->source_position(), "Duplicated symbol: %s", var->identifier()->data());
                return -1;
            }
        }
        
        std::vector<Type *> types;
        for (auto expr : node->initilaizers()) {
            if (Reduce(expr, &types) < 0) {
                return -1;
            }
        }
        if (types.size() != node->variables_size()) {
            Feedback()->Printf(node->source_position(), "Different declaration numbers and initilizer numbers, %zd vs %zd",
                               node->variables_size(), types.size());
            return -1;
        }
        
        for (size_t i = 0; i < node->variables_size(); i++) {
            auto var = node->variable(i);
            if (!var->type()) {
                var->set_type(types[i]);
            } else {
                if (!AcceptableOrLink(var->mutable_type(), types[i])) {
                    Feedback()->Printf(var->source_position(), "LVal is not acceptable from RVal");
                    return -1;
                }
            }
        }
        return Return(Unit());
    }

    int VisitAssignment(Assignment *node) override { UNREACHABLE(); }
    int VisitStructDefinition(StructDefinition *node) override { UNREACHABLE(); }
    int VisitClassDefinition(ClassDefinition *node) override { UNREACHABLE(); }
    int VisitAnnotationDefinition(AnnotationDefinition *node) override { UNREACHABLE(); }
    int VisitInterfaceDefinition(InterfaceDefinition *node) override { UNREACHABLE(); }
    int VisitFunctionDeclaration(FunctionDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotationDeclaration(AnnotationDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotation(Annotation *node) override { UNREACHABLE(); }
    int VisitBreak(Break *node) override { UNREACHABLE(); }
    int VisitContinue(Continue *node) override { UNREACHABLE(); }
    int VisitReturn(Return *node) override { UNREACHABLE(); }
    int VisitThrow(Throw *node) override { UNREACHABLE(); }
    int VisitRunCoroutine(RunCoroutine *node) override { UNREACHABLE(); }
    int VisitWhileLoop(WhileLoop *ast) override { UNREACHABLE(); }
    int VisitUnlessLoop(UnlessLoop *node) override { UNREACHABLE(); }
    int VisitForeachLoop(ForeachLoop *node) override { UNREACHABLE(); }
    int VisitStringTemplate(StringTemplate *node) override { UNREACHABLE(); }
    int VisitInstantiation(Instantiation *node) override { UNREACHABLE(); }
    int VisitOr(Or *node) override { UNREACHABLE(); }
    int VisitAdd(Add *node) override { UNREACHABLE(); }
    int VisitAnd(And *node) override { UNREACHABLE(); }
    int VisitDiv(Div *node) override { UNREACHABLE(); }
    int VisitDot(Dot *node) override { UNREACHABLE(); }
    int VisitMod(Mod *node) override { UNREACHABLE(); }
    int VisitMul(Mul *node) override { UNREACHABLE(); }
    int VisitNot(Not *node) override { UNREACHABLE(); }
    int VisitSub(Sub *node) override { UNREACHABLE(); }
    int VisitLess(Less *node) override { UNREACHABLE(); }
    int VisitRecv(Recv *node) override { UNREACHABLE(); }
    int VisitSend(Send *node) override { UNREACHABLE(); }
    int VisitEqual(Equal *node) override { UNREACHABLE(); }
    int VisitCalling(Calling *node) override { UNREACHABLE(); }
    int VisitCasting(Casting *node) override { UNREACHABLE(); }
    int VisitGreater(Greater *node) override { UNREACHABLE(); }
    int VisitTesting(Testing *node) override { UNREACHABLE(); }
    int VisitNegative(Negative *node) override { UNREACHABLE(); }
    int VisitIdentifier(Identifier *node) override { UNREACHABLE(); }
    int VisitNotEqual(NotEqual *node) override { UNREACHABLE(); }
    int VisitBitwiseOr(BitwiseOr *node) override { UNREACHABLE(); }
    int VisitLessEqual(LessEqual *node) override { UNREACHABLE(); }
    int VisitBitwiseAnd(BitwiseAnd *node) override { UNREACHABLE(); }
    int VisitBitwiseShl(BitwiseShl *node) override { UNREACHABLE(); }
    int VisitBitwiseShr(BitwiseShr *node) override { UNREACHABLE(); }
    int VisitBitwiseXor(BitwiseXor *node) override { UNREACHABLE(); }
    int VisitF32Literal(F32Literal *node) override { UNREACHABLE(); }
    int VisitF64Literal(F64Literal *node) override { UNREACHABLE(); }
    int VisitI64Literal(I64Literal *node) override { UNREACHABLE(); }
    int VisitIndexedGet(IndexedGet *node) override { UNREACHABLE(); }
    int VisitIntLiteral(IntLiteral *node) override { UNREACHABLE(); }
    int VisitU64Literal(U64Literal *node) override { UNREACHABLE(); }
    int VisitBoolLiteral(BoolLiteral *node) override { UNREACHABLE(); }
    int VisitUnitLiteral(UnitLiteral *node) override { UNREACHABLE(); }
    int VisitEmptyLiteral(EmptyLiteral *node) override { UNREACHABLE(); }
    int VisitGreaterEqual(GreaterEqual *node) override { UNREACHABLE(); }
    int VisitIfExpression(IfExpression *node) override { UNREACHABLE(); }
    int VisitLambdaLiteral(LambdaLiteral *node) override { UNREACHABLE(); }
    int VisitStringLiteral(StringLiteral *node) override { UNREACHABLE(); }
    int VisitWhenExpression(WhenExpression *node) override { UNREACHABLE(); }
    int VisitBitwiseNegative(BitwiseNegative *node) override { UNREACHABLE(); }
    int VisitArrayInitializer(ArrayInitializer *node) override { UNREACHABLE(); }
    int VisitObjectDeclaration(ObjectDeclaration *node) override { UNREACHABLE(); }
    int VisitUIntLiteral(UIntLiteral *node) override { return Return(node->type()); }
    int VisitTryCatchExpression(TryCatchExpression *node) override { UNREACHABLE(); }
    
    
    int Return(Type *type) {
        results_.push(DCHECK_NOTNULL(type));
        return 1;
    }
    
    int Return(const std::vector<Type *> &types) {
        for (auto ty : types) {
            Return(ty);
        }
        return static_cast<int>(types.size());
    }
    
    int Reduce(AstNode *node, std::vector<Type *> *receiver = nullptr) {
        int nt = node->Accept(this);
        if (fail()) {
            return -1;
        }
        if (receiver) {
            receiver->resize(receiver->size() + nt);
        }
        while (nt--) {
            if (receiver) {
                (*receiver)[receiver->size() + nt] = results_.top();
            }
            results_.pop();
        }
        return nt;
    }
    
    bool AcceptableOrLink(Type **lhs, const Type *rhs) {
        bool unlinked = false;
        bool ok = (*lhs)->Acceptable(rhs, &unlinked);
        if (!unlinked) {
            return ok;
        }
        *lhs = LinkType(*lhs);
        return (*lhs)->Acceptable(rhs, &unlinked);
    }
    
    Type *LinkType(Type *type) {
        return type->Link(std::bind(&TypeReducingVisitor::FindType, this, std::placeholders::_1));
    }
    
    Type *FindType(const Symbol *name) {
        auto found = FindSymbol(!name->prefix_name() ? "" : name->prefix_name()->ToSlice(), name->name()->ToSlice());
        if (!found) {
            Feedback()->Printf(name->source_position(), "Symbol not found!");
            return nullptr;
        }
        
        switch (found->kind()) {
            case Node::kClassDefinition:
                return new (arena_) ClassType(arena_, found->AsClassDefinition(), name->source_position());
            case Node::kStructDefinition:
                return new (arena_) StructType(arena_, found->AsStructDefinition(), name->source_position());
            case Node::kInterfaceType:
                return new (arena_) InterfaceType(arena_, found->AsInterfaceDefinition(), name->source_position());
            case Node::kObjectDeclaration:
                UNREACHABLE();
                break;
            default:
                Feedback()->Printf(name->source_position(), "%s is not a type", name->ToString().c_str());
                break;
        }
        return nullptr;
    }
    
    Statement *FindSymbol(std::string_view prefix, std::string_view name) {
        if (prefix.empty()) {
            return std::get<0>(location_->FindSymbol(name));
        }
        std::string full_name(prefix.data(), prefix.size());
        full_name.append(".").append(name.data(), name.size());
        if (auto iter = global_symbols_.find(full_name); iter == global_symbols_.end()) {
            return nullptr;
        } else {
            return iter->second.ast;
        }
    }
    
    Type *Unit() {
        if (!unit_) {
            unit_ = new (arena_) Type(arena_, Type::kType_unit, {0,0});
        }
        return unit_;
    }
    
    bool fail() { return status_.fail(); }
    
    SyntaxFeedback *Feedback() {
        auto file = location_->NearlyFileUnitScope();
        auto pkg = file->NearlyPackageScope();
        error_feedback_->set_file_name(file->file_unit()->file_name()->ToString());
        error_feedback_->set_package_name(pkg->pkg()->name()->ToString());
        status_ = ERR_CORRUPTION("Type checking fail!");
        return error_feedback_;
    }
    
    SyntaxFeedback *feedback() { return error_feedback_; }
    
    
    struct Global {
        String    *symbol;
        Statement *ast;
        Package   *owns;
    };
    
    base::Arena *const arena_;
    SyntaxFeedback *const error_feedback_;
    NamespaceScope *location_ = nullptr;
    Package *entry_;
    base::Status status_;
    std::unordered_map<std::string_view, Global> global_symbols_;
    std::stack<Type *> results_;
    Type *unit_ = nullptr;
}; // class TypeReducingVisitor

TypeReducingVisitor::TypeReducingVisitor(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback)
    : arena_(arena)
    , error_feedback_(error_feedback)
    , entry_(entry) {
}


base::Status ReducePackageDependencesType(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback) {
    TypeReducingVisitor visitor(entry, arena, error_feedback);
    return visitor.Reduce();
}


} // namespace cpl

} // namespace yalx
