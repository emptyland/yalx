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
        track_.clear();
        return Recursive(entry_, std::bind(&Package::Accept, std::placeholders::_1, this));
    }
    
    void MoveGlobalSymbols(std::unordered_map<std::string_view, GlobalSymbol> *receiver) {
        *receiver = std::move(global_symbols_);
    }
private:
    base::Status Recursive(Package *root, std::function<void(Package *)> &&callback) {
        if (root->IsTerminator()) {
            callback(root);
            return status_;
        }
        for (auto pkg : root->dependences()) {
            if (auto rs = Recursive(pkg, std::move(callback)); rs.fail()) {
                return rs;
            }
        }
        callback(root);
        return status_;
    }
    
    void PreparePackage(Package *pkg) {
        if (Track(pkg)) {
            return;
        }
        printd("prepare package: %s", pkg->name()->data());
        error_feedback_->set_package_name(pkg->name()->ToString());
        PrepareInterfaces(pkg);
        PrepareClasses(pkg);
        PrepareOthers(pkg);
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
                        auto any = FindGlobal("yalx/lang:lang", "Any"); // Any class
                        assert(any.IsFound());
                        clazz->set_base_of(DCHECK_NOTNULL(any.ast->AsClassDefinition()));
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
    
    void PrepareOthers(Package *node) {
        for (auto file : node->source_files()) {
            for (auto var : file->vars()) {
                for (int i = 0; i < var->ItemSize(); i++) {
                    auto item = var->AtItem(i);
                    auto symbol = FindOrInsertGlobal(node, item->Identifier()->ToSlice(), item);
                    if (symbol.IsFound()) {
                        Feedback()->Printf(item->source_position(), "Duplicated symbol: %s", item->Identifier()->data());
                        return;
                    }
                }
            }
            
            for (auto fun : file->funs()) {
                auto symbol = FindOrInsertGlobal(node, fun->name()->ToSlice(), fun);
                if (symbol.IsFound()) {
                    Feedback()->Printf(fun->source_position(), "Duplicated symbol: %s", fun->name()->data());
                    return;
                }
            }
        }
    }
    
    int VisitPackage(Package *node) override {
        if (Track(node)) {
            return 0; // Skip if has processed
        }
        PackageScope scope(&location_, node, &global_symbols_);
        error_feedback_->set_package_name(node->name()->ToString());
        
        printd("process package: %s", node->name()->data());
        for (auto file_scope : scope.files()) {
            file_scope->Enter();
            error_feedback_->set_file_name(file_scope->file_unit()->file_name()->ToString());

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
            auto pkg_scope = location_->NearlyPackageScope();
            if (pkg_scope->Track(ast)) {
                continue;
            }
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
        if (!node->owns()) {
            for (auto var : node->variables()) {
                auto duplicated = location_->FindOrInsertSymbol(var->identifier()->ToSlice(), var);
                if (duplicated) {
                    Feedback()->Printf(var->source_position(), "Duplicated symbol: %s", var->identifier()->data());
                    return -1;
                }
            }
        }
        
        std::vector<Type *> types;
        for (auto expr : node->initilaizers()) {
            if (Reduce(expr, &types) < 0) {
                return -1;
            }
        }
        //if (!node->Type()) {
        if (types.size() != node->variables_size()) {
            Feedback()->Printf(node->source_position(),
                               "Different declaration numbers and initilizer numbers, %zd vs %zd",
                               node->variables_size(), types.size());
            return -1;
        }
        //}
        
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
    
    int VisitClassDefinition(ClassDefinition *node) override {
        auto classes_count = 0;
        for (auto i = 0; i < node->concepts_size(); i++) {
            auto concept = LinkType(node->concept(i));
            if (!concept) {
                return -1;
            }
            
            if (concept->IsClassType()) {
                // Only one class concept!
                if (classes_count++ > 0) {
                    Feedback()->Printf(concept->source_position(), "Concept classes number > 1");
                    return -1;
                }
                node->set_base_of(concept->AsClassType()->definition());
            } else {
                if (!concept->IsInterfaceType()) {
                    Feedback()->Printf(concept->source_position(), "Concept must be a interface");
                    return -1;
                }
            }
            *node->mutable_concept(i) = concept;
        }
        
        // Default base class is Any;
        if (node->base_of()) {
            auto symbol = FindGlobal("yalx/lang:lang", "Any");
            if (symbol.IsNotFound()) {
                Feedback()->Printf(node->source_position(), "lang.Any class not found");
                return -1;
            }
            node->set_base_of(DCHECK_NOTNULL(symbol.ast->AsClassDefinition()));
        }
        
        DataDefinitionScope scope(&location_, node);
        // Into class scope:
        for (int i = 0; i < node->fields_size(); i++) {
            auto field = node->field(i);
            if (field.in_constructor) {
                auto name = field.declaration->Identifier();
                auto dup = scope.FindOrInsertSymbol(name->ToSlice(), field.declaration);
                if (dup) {
                    Feedback()->Printf(field.declaration->source_position(), "duplicated class field: `%s'",
                                       name->data());
                    return -1;
                }
                
                if (!LinkType(field.declaration->Type())) {
                    return -1;
                }
                continue;
            }
            if (Reduce(field.declaration) < 0) {
                return -1;
            }
        }
        
        for (auto method : node->methods()) {
            if (Reduce(method) < 0) {
                return -1;
            }
        }
        
        return Return(Unit());
    }
    
    int VisitFunctionDeclaration(FunctionDeclaration *node) override {
        FunctionScope scope(&location_, node);
        
        // Install `this' variable if needed
        if (auto data_scope = location_->NearlyDataDefinitionScope()) {
            scope.FindOrInsertSymbol("this", data_scope->ThisStub(arena_));
        }
        
        auto prototype = node->prototype();
        for (auto item : prototype->params()) {
            auto param = static_cast<VariableDeclaration::Item *>(item);
            scope.FindOrInsertSymbol(param->identifier()->ToSlice(), param);
        }
        if (prototype->vargs()) {
            // TODO:
            UNREACHABLE();
        }
        if (node->IsNative() || node->IsAbstract()) {
            return Return(Unit());
        }
        
        std::vector<Type *> receiver;
        int nrets = Reduce(node->body(), &receiver);
        if (nrets < 0) {
            return -1;
        }
        
        if (prototype->return_types_size() > 0) {
            if (prototype->return_types_size() != nrets) {
                Feedback()->Printf(node->source_position(), "unexpected return val numbers, %d, %zd", nrets,
                                   prototype->return_types_size());
                return -1;
            }
            
            for (auto i = 0; i < prototype->return_types_size(); i++) {
                bool unlinked = false;
                if (!receiver[i]->Acceptable(prototype->return_type(i), &unlinked)) {
                    Feedback()->Printf(node->source_position(), "return type not accepted, %s <= %s",
                                       prototype->return_type(i)->ToString().c_str(),
                                       receiver[i]->ToString().c_str());
                    return -1;
                }
            }
        } else {
            assert(prototype->return_types_size() == 0);
            for (auto ty : receiver) {
                prototype->mutable_return_types()->push_back(ty);
            }
        }
        
        return Return(Unit());
    }
    
    int VisitIdentifier(Identifier *node) override {
        //node->name()->data();
        auto [ast, ns] = location_->FindSymbol(node->name()->ToSlice());
        if (!ast) {
            Feedback()->Printf(node->source_position(), "symbol: `%s' not found", node->name()->data());
            return -1;
        }
        return ProcessSymbol(node->name()->data(), node, ast);
    }
    
    int VisitDot(Dot *node) override {
        if (node->primary()->IsIdentifier()) {
            auto id = node->primary()->AsIdentifier();
            if (MaybePackageName(id)) {
                auto file_scope = location_->NearlyFileUnitScope();
                auto symbol = file_scope->FindExportSymbol(id->name()->ToSlice(), node->field()->ToSlice());
                if (!symbol) {
                    Feedback()->Printf(node->source_position(), "symbol: %s.%s not found", id->name()->data(),
                                       node->field()->data());
                    return -1;
                }
                return ProcessSymbol(id->name()->data(), node, symbol);
            }
        }
        std::vector<Type *> types;
        if (Reduce(node->primary(), &types) < 0) {
            return -1;
        }
        // TODO:
        UNREACHABLE();
        return -1;
    }
    
    int ProcessSymbol(const char *name, Statement *node, Statement *ast) {
        auto pkg_scope = location_->NearlyPackageScope();
        if (pkg_scope->HasNotTracked(ast)) {
            auto file_scope = location_->NearlyFileUnitScope();
            AstNode *owns = nullptr, *sym = ast;
            if (Declaration::Is(ast)) {
                owns = down_cast<Declaration>(ast)->owns();
            } else if (Definition::Is(ast)) {
                owns = down_cast<Definition>(ast)->owns();
            } else {
                sym = down_cast<VariableDeclaration::Item>(ast)->owns();
                owns = down_cast<VariableDeclaration::Item>(ast)->owns()->owns();
            }
            if (owns) {
                if (owns != file_scope->file_unit()) {
                    printd("nested reduce: %s", down_cast<FileUnit>(owns)->file_name()->data());
                    //auto saved = location_;
                    //location_ = nullptr;
                    auto scope = DCHECK_NOTNULL(pkg_scope->FindFileUnitScopeOrNull(owns));
                    assert(scope != file_scope);
                    scope->Enter();
                    auto rs = Reduce(sym);
                    scope->Exit();
                    //location_ = saved;
                    if (rs < 0) {
                        return -1;
                    }
                } else {
                    if (auto rs = Reduce(sym); rs < 0) {
                        return -1;
                    }
                }
            }
        }
        
        switch (ast->kind()) {
            case Node::kFunctionDeclaration:
                return Return(ast->AsFunctionDeclaration()->prototype());
            case Node::kObjectDeclaration:
                return Return(ast->AsObjectDeclaration()->Type());
            case Node::kVariableDeclaration:
                assert(ast->AsVariableDeclaration()->ItemSize() == 1);
                return Return(ast->AsVariableDeclaration()->Type());
            default: {
                if (auto var = down_cast<VariableDeclaration::Item>(ast)) {
                    assert(var->type() != nullptr);
                    return Return(var->type());
                }
            } break;
        }
        Feedback()->Printf(node->source_position(), "symbol: `%s' not found", name);
        return -1;
    }


    int VisitAssignment(Assignment *node) override { UNREACHABLE(); }
    int VisitStructDefinition(StructDefinition *node) override { UNREACHABLE(); }
    
    int VisitAnnotationDefinition(AnnotationDefinition *node) override { UNREACHABLE(); }
    int VisitInterfaceDefinition(InterfaceDefinition *node) override { UNREACHABLE(); }
    
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
    int VisitIntLiteral(IntLiteral *node) override { return Return(node->type()); }
    int VisitU64Literal(U64Literal *node) override { UNREACHABLE(); }
    int VisitBoolLiteral(BoolLiteral *node) override { UNREACHABLE(); }
    int VisitUnitLiteral(UnitLiteral *node) override { return Return(Unit()); }
    int VisitEmptyLiteral(EmptyLiteral *node) override { UNREACHABLE(); }
    int VisitGreaterEqual(GreaterEqual *node) override { UNREACHABLE(); }
    int VisitIfExpression(IfExpression *node) override { UNREACHABLE(); }
    int VisitLambdaLiteral(LambdaLiteral *node) override { UNREACHABLE(); }
    int VisitStringLiteral(StringLiteral *node) override { return Return(node->type()); }
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
        const int nrets = node->Accept(this);
        if (fail() || nrets < 0) {
            return -1;
        }
        if (receiver) {
            receiver->resize(receiver->size() + nrets);
        }
        int i = nrets;
        while (i--) {
            if (receiver) {
                (*receiver)[i] = results_.top();
            }
            results_.pop();
        }
        return nrets;
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
        std::string full_name = owns->path()->ToString();
        full_name.append(":").append(owns->name()->ToString()).append(".").append(name.data(), name.size());
        
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
        printd("insert global: %s", symbol.symbol->data());
        
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
    
    bool Track(Package *pkg) {
        if (auto iter = track_.find(pkg); iter != track_.end()) {
            return true;
        }
        track_.insert(pkg);
        return false;
    }
    
    bool MaybePackageName(Identifier *id) {
        auto [ast, ns] = location_->FindSymbol(id->name()->ToSlice());
        return !ast;
    }
    
    base::Arena *const arena_;
    SyntaxFeedback *const error_feedback_;
    NamespaceScope *location_ = nullptr;
    Package *entry_;
    std::set<Package *> track_;
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
    
    auto rs = visitor.Reduce();
    if (rs.ok()) {
        visitor.MoveGlobalSymbols(symbols);
    }
    return rs;
}


} // namespace cpl

} // namespace yalx
