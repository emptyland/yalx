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
        Recursive(entry_, std::bind(&TypeReducingVisitor::PreparePackage, this, std::placeholders::_1));
        if (fail()) {
            return status_;
        }
        return Recursive(entry_, std::bind(&Package::Accept, std::placeholders::_1, this));
    }
    
    void MoveGlobalSymbols(std::unordered_map<std::string_view, GlobalSymbol> *receiver) {
        *receiver = std::move(global_symbols_);
    }
private:
    base::Status Recursive(Package *root, std::function<void(Package *)> &&callback) {
        callback(root);
        if (fail()) {
            return status_;
        }
        if (root->IsTerminator()) {
            return status_;
        }
        for (auto pkg : root->dependences()) {
            if (auto rs = Recursive(pkg, std::move(callback)); rs.fail()) {
                return rs;
            }
        }
        return status_;
    }
    
    void PreparePackage(Package *pkg) {
        error_feedback_->set_package_name(pkg->name()->ToString());
        PrepareInterfaces(pkg);
        PrepareClasses(pkg);
    }
    
    void PrepareInterfaces(Package *node) {
        for (auto file : node->source_files()) {
            error_feedback_->set_file_name(file->file_name()->ToString());
            for (auto stmt : file->interfaces()) {
                auto symbol = FindOrInsertGlobal(node, stmt->name()->ToSlice(), stmt);
                if (symbol.IsFound()) {
                    Feedback()->Printf(stmt->source_position(), "Duplicated symbol: %s", stmt->name()->data());
                    return;
                }
            }
        }
    }
    
    void PrepareClasses(Package *node) {
        for (auto file : node->source_files()) {
            error_feedback_->set_file_name(file->file_name()->ToString());
            for (auto stmt : file->class_defs()) {
                auto symbol = FindOrInsertGlobal(node, stmt->name()->ToSlice(), stmt);
                if (symbol.IsFound()) {
                    Feedback()->Printf(stmt->source_position(), "Duplicated symbol: %s", stmt->name()->data());
                    return;
                }
            }
        }
        
        for (auto file : node->source_files()) {
            for (auto clazz : file->class_defs()) {
                auto base_ast = !clazz->super_calling() ? nullptr : clazz->super_calling()->callee();
                if (!base_ast) {
                    if (node->path()->ToString() != "yalx/lang" && clazz->name()->ToString() != "Any") {
                        auto any = FindGlobal("lang", "Any"); // Any class
                        assert(any.IsFound());
                        clazz->set_base_of(DCHECK_NOTNULL(any.ast->AsClassDefinition()));
                    }
                    continue;
                }
                
                if (base_ast->IsInstantiation()) {
                    auto symbol = GenericsInstantiate(base_ast->AsInstantiation());
                    if (symbol.IsNotFound()) {
                        return;
                    }
                    continue;
                }
                
                auto [prefix, name] = GetSymbol(base_ast, node->name()->ToSlice());
                auto symbol = FindGlobal(prefix, name);
                if (symbol.IsNotFound() || !symbol.ast->IsClassDefinition()) {
                    Feedback()->Printf(base_ast->source_position(), "Base class: %s.%s not found", prefix.data(),
                                       name.data());
                    return;
                }
                clazz->set_base_of(symbol.ast->AsClassDefinition());
            }
        }
    }
    
    int VisitPackage(Package *node) override {
        PackageScope scope(&location_, node);
        error_feedback_->set_package_name(node->name()->ToString());
        
        for (auto file_scope : scope.files()) {
            file_scope->Enter();
            // TODO: INIT
            error_feedback_->set_file_name(file_scope->file_unit()->file_name()->ToString());
            
//            if (Reduce(file_scope->file_unit()) < 0) {
//                file_scope->Exit();
//                return -1;
//            }
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
    int VisitIntLiteral(IntLiteral *node) override { return Return(I32()); }
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
    
    
    GlobalSymbol GenericsInstantiate(Instantiation *inst) {
        auto symbol = FindGlobal(inst->primary());
        if (!symbol.ast) {
            auto [prefix, name] = GetSymbol(inst->primary());
            Feedback()->Printf(inst->source_position(), "Symbol: %s.%s not found", prefix.data(), name.data());
            return GlobalSymbol::NotFound();
        }
        
        UNREACHABLE();
        return GlobalSymbol::NotFound(); // TODO:
    }
    
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
        if (fail() || nt < 0) {
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
    
    GlobalSymbol FindOrInsertGlobal(Package *owns, std::string_view name, Statement *ast) {
        std::string full_name = owns->name()->ToString();
        full_name.append(".").append(name.data(), name.size());
        
        auto iter = global_symbols_.find(full_name);
        if (iter != global_symbols_.end()) {
            return iter->second;
        }
        GlobalSymbol symbol = {
            .symbol = String::New(arena_, full_name),
            .owns = owns,
            .ast = ast,
        };
        global_symbols_[symbol.symbol->ToSlice()] = symbol;
        return GlobalSymbol::NotFound();
    }
    
    GlobalSymbol FindGlobal(Expression *expr) const {
        auto [prefix, name] = GetSymbol(expr, location_->NearlyPackageScope()->pkg()->name()->ToSlice());
        return FindGlobal(prefix, name);
    }
    
    GlobalSymbol FindGlobal(std::string_view prefix, std::string_view name) const {
        std::string full_name(prefix.data(), prefix.size());
        full_name.append(".").append(name.data(), name.size());
        if (auto iter = global_symbols_.find(full_name); iter == global_symbols_.end()) {
            return {nullptr, nullptr, nullptr};
        } else {
            return iter->second;
        }
    }
    
    static std::tuple<std::string_view, std::string_view> GetSymbol(Expression *expr,
                                                                    std::string_view default_prefix = "") {
        if (expr->IsIdentifier()) {
            return std::make_tuple(default_prefix, expr->AsIdentifier()->name()->ToSlice());
        } else {
            assert(expr->IsDot());
            auto prefix = DCHECK_NOTNULL(expr->AsDot()->primary()->AsIdentifier());
            auto name = expr->AsDot()->field();
            return std::make_tuple(prefix->name()->ToSlice(), name->ToSlice());
        }
    }
    
    Type *Unit() {
        if (!unit_) {
            unit_ = new (arena_) Type(arena_, Type::kType_unit, {0,0});
        }
        return unit_;
    }
    
    Type *I32() {
        if (!i32_) {
            i32_ = new (arena_) Type(arena_, Type::kType_i32, {0,0});
        }
        return i32_;
    }
    
    bool fail() { return status_.fail(); }
    
    SyntaxFeedback *Feedback() {
        auto file = !location_ ? nullptr : location_->NearlyFileUnitScope();
        auto pkg = !file ? nullptr : file->NearlyPackageScope();
        if (file) {
            error_feedback_->set_file_name(file->file_unit()->file_name()->ToString());
        }
        if (pkg) {
            error_feedback_->set_package_name(pkg->pkg()->name()->ToString());
        }
        status_ = ERR_CORRUPTION("Type checking fail!");
        return error_feedback_;
    }
    
    SyntaxFeedback *feedback() { return error_feedback_; }
    
    base::Arena *const arena_;
    SyntaxFeedback *const error_feedback_;
    NamespaceScope *location_ = nullptr;
    Package *entry_;
    base::Status status_;
    std::unordered_map<std::string_view, GlobalSymbol> global_symbols_;
    std::stack<Type *> results_;
    Type *unit_ = nullptr;
    Type *i32_ = nullptr;
}; // class TypeReducingVisitor

TypeReducingVisitor::TypeReducingVisitor(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback)
    : arena_(arena)
    , error_feedback_(error_feedback)
    , entry_(entry) {
}


base::Status ReducePackageDependencesType(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback,
                                          std::unordered_map<std::string_view, GlobalSymbol> *symbols) {
    TypeReducingVisitor visitor(entry, arena, error_feedback);
    visitor.MoveGlobalSymbols(symbols);
    return visitor.Reduce();
}


} // namespace cpl

} // namespace yalx
