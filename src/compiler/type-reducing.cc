#include "compiler/type-reducing.h"
#include "compiler/generics-instantiating.h"
#include "compiler/syntax-feedback.h"
#include "compiler/scope.h"
#include "compiler/ast.h"
#include <stack>

namespace yalx {

namespace cpl {

class TypeReducingVisitor : public AstVisitor {
public:
    TypeReducingVisitor(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback);
    
    // package_scopes_
    ~TypeReducingVisitor() {
        for (auto pair : package_scopes_) {
            delete pair.second;
        }
    }
    
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
    class InstantiatingResolver : public GenericsInstantiating::Resolver {
    public:
        InstantiatingResolver(TypeReducingVisitor *owns): owns_(owns) {}
        Statement *Find(std::string_view prefix, std::string_view name) override {
            auto file_scope = owns_->location_->NearlyFileUnitScope();
            if (prefix.empty()) {
                return file_scope->FindLocalSymbol(name);
            } else {
                return file_scope->FindExportSymbol(prefix, name);
            }
        }
        Statement *FindOrInsert(std::string_view prefix, std::string_view name, Statement *ast) override {
            auto file_scope = owns_->location_->NearlyFileUnitScope();
            if (prefix.empty()) {
                return file_scope->FindOrInsertSymbol(name, ast);
            } else {
                return file_scope->FindOrInsertExportSymbol(prefix, name, ast);
            }
        }
        void Enter(Statement *ast) override {
            auto [owns, _] = ast->Owns(true/*force*/);
            auto pkg = ast->Pack(true/*force*/);
            auto current_file_scope = owns_->location_->NearlyFileUnitScope();
            auto current_pkg_scope = current_file_scope->NearlyPackageScope();
            assert(owns->IsFileUnit());
            auto pkg_scope = owns_->EnsurePackageScope(pkg);
            auto file_scope = DCHECK_NOTNULL(pkg_scope->FindFileUnitScopeOrNull(owns));
            if (pkg != current_pkg_scope->pkg()) {
                pkg_scope->Enter();
                scopes_.push(pkg_scope);
                file_scope->Enter();
                scopes_.push(file_scope);
                depth_.push(2);
            } else if (owns != current_file_scope->file_unit()) {
                file_scope->Enter();
                scopes_.push(file_scope);
                depth_.push(1);
            } else {
                depth_.push(0);
            }
        }
        void Exit(Statement *) override {
            int depth = depth_.top();
            depth_.pop();
            for (int i = 0; i < depth; i++) {
                scopes_.top()->Exit();
                scopes_.pop();
            }
        }
    private:
        TypeReducingVisitor *owns_;
        std::stack<int> depth_;
        std::stack<NamespaceScope *> scopes_;
    }; // class InstantiatingResolver
    
    
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
        NewPackageScopeIfNeeded(pkg);
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

//                auto [prefix, name] = GetSymbol(base_ast, node->name()->ToSlice());
//                auto symbol = FindGlobal(prefix, name);
//                if (symbol.IsNotFound() || !symbol.ast->IsClassDefinition()) {
//                    Feedback()->Printf(base_ast->source_position(), "Base class: %s.%s not found", prefix.data(),
//                                       name.data());
//                    return;
//                }
//                clazz->set_base_of(symbol.ast->AsClassDefinition());
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
        NamespaceScope::Keeper<PackageScope> keeper(EnsurePackageScope(node));
        error_feedback_->set_package_name(node->name()->ToString());
        
        printd("process package: %s", node->name()->data());
        for (auto file_scope : keeper.ns()->files()) {
            NamespaceScope::Keeper<FileUnitScope> file_keeper(file_scope);
            error_feedback_->set_file_name(file_scope->file_unit()->file_name()->ToString());

            if (Reduce(file_scope->file_unit()) < 0) {
                return -1;
            }
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
        if (!node->generic_params().empty()) {
            return Return(Unit());
        }
        if (node->super_calling()) {
            auto super_call = node->super_calling();
            Statement *symbol = nullptr;
            switch (super_call->callee()->kind()) {
                case Node::kIdentifier:
                    symbol = ResolveIdSymbol("base class", super_call->callee()->AsIdentifier(), Node::kClassDefinition);
                    break;
                case Node::kDot:
                    symbol = ResolveDotSymbol("base class", super_call->callee()->AsDot(), Node::kClassDefinition);
                    break;
                case Node::kInstantiation:
                    symbol = Instantiate("base class", super_call->callee()->AsInstantiation(), Node::kClassDefinition);
                    break;
                default:
                    break;
            }
            if (!symbol) {
                return -1;
            }
            node->set_base_of(symbol->AsClassDefinition());
        }
        
        auto classes_count = 0;
        for (auto i = 0; i < node->concepts_size(); i++) {
            auto concept = LinkType(node->concept(i));
            if (!concept) {
                return -1;
            }
            
            if (!concept->IsInterfaceType()) {
                Feedback()->Printf(concept->source_position(), "Concept must be a interface");
                return -1;
            }
            *node->mutable_concept(i) = concept;
        }
        
        // Default base class is Any;
        if (!node->base_of()) {
            auto any_class = FindGlobal("yalx/lang:lang", "Any");
            if (any_class.IsNotFound()) {
                Feedback()->Printf(node->source_position(), "lang.Any class not found");
                return -1;
            }
            // Don't set base class of Any class
            if (any_class.ast != node) {
                node->set_base_of(DCHECK_NOTNULL(any_class.ast->AsClassDefinition()));
            }
        }
        
        DataDefinitionScope scope(&location_, node);
        // Into class scope:
        //std::map<std::string_view, Statement *> in_class_symols_;
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
                
                if (!LinkType(DCHECK_NOTNULL(field.declaration->Type()))) {
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
            
            if (method->decoration() == FunctionDeclaration::kOverride) {
                UNREACHABLE(); // check override
            }
        }
        
        return Return(Unit());
    }
    
    Statement *Instantiate(const char *info, Instantiation *ast, Node::Kind kind) {
        std::vector<Type *> types;
        for (auto arg : ast->generic_args()) {
            auto linked = LinkType(arg);
            if (!linked) {
                return nullptr;
            }
            types.push_back(linked);
        }
        
        Statement *symbol = nullptr;
        if (auto id = ast->primary()->AsIdentifier()) {
            symbol = ResolveIdSymbol(info, id, kind, types);
        } else if (auto dot = ast->primary()->AsDot()) {
            symbol = ResolveDotSymbol(info, dot, kind, types);
        } else {
            UNREACHABLE();
        }
        if (!symbol) {
            return nullptr;
        }
        //hook(symbol);
        if (symbol->IsNotTemplate()) {
            Feedback()->Printf(ast->source_position(), "%s is template, need instantiation", info);
            return nullptr;
        }
        
        InstantiatingResolver resover(this);
        auto rs = GenericsInstantiating::Instantiate(nullptr, symbol, arena_, error_feedback_, &resover,
                                                     types.size(), &types[0], &symbol);
        if (rs.fail()) {
            return nullptr;
        }
        if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
            return nullptr;
        }
        return symbol;
    }
    
    Statement *ResolveIdSymbol(const char *info, Identifier *ast, Node::Kind kind,
                               const std::vector<Type *> &types = {}) {
        auto name = ast->name();
        auto file_scope = location_->NearlyFileUnitScope();
        Statement *symbol = nullptr;
        if (types.empty()) {
            symbol = file_scope->FindLocalSymbol(name->ToSlice());
        } else {
//            std::string buf(name->ToString());
//            buf.append("<");
//            for (size_t i = 0; i < types.size(); i++) {
//                if (i > 0) {
//                    buf.append(",");
//                }
//                buf.append(types[i]->ToString());
//            }
//            buf.append(">");
            symbol = file_scope->FindLocalSymbol(MakeFullInstantiatingName(name->ToSlice(), types.size(), &types[0]));
            if (!symbol) {
                symbol = file_scope->FindLocalSymbol(name->ToSlice());
            }
        }
        if (!symbol || symbol->kind() != kind) {
            Feedback()->Printf(ast->source_position(), "%s: %s not found", info, name->data());
            return nullptr;
        }
        if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
            return nullptr;
        }
        return symbol;
    }
    
    Statement *ResolveDotSymbol(const char *info, Dot *ast, Node::Kind kind,
                                const std::vector<Type *> &types = {}) {
        auto prefix = DCHECK_NOTNULL(ast->primary()->AsIdentifier());
        auto name = ast->field();
        auto file_scope = location_->NearlyFileUnitScope();
        Statement *symbol = nullptr;
        if (types.empty()) {
            symbol = file_scope->FindExportSymbol(prefix->name()->ToSlice(), name->ToSlice());
        } else {
            symbol = file_scope->FindExportSymbol(prefix->name()->ToSlice(),
                                                  MakeFullInstantiatingName(name->ToSlice(), types.size(), &types[0]));
            if (!symbol) {
                symbol = file_scope->FindExportSymbol(prefix->name()->ToSlice(), name->ToSlice());
            }
        }
        if (!symbol || symbol->kind() != kind) {
            Feedback()->Printf(ast->source_position(), "%s: %s.%s not found", info, prefix->name()->data(),
                               name->data());
            return nullptr;
        }
        if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
            return nullptr;
        }
        return symbol;
    }
    
    int VisitFunctionDeclaration(FunctionDeclaration *node) override {
        if (auto duplicated = location_->FindOrInsertSymbol(node->name()->ToSlice(), node)) {
            Feedback()->Printf(node->source_position(), "Duplicated function name: %s", node->name()->data());
            return -1;
        }
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
        return ReduceDependencySymbolIfNeeded(node->name()->data(), node, ast);
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
                return ReduceDependencySymbolIfNeeded(id->name()->data(), node, symbol);
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

    
    int ReduceDependencySymbolIfNeeded(const char *name, Statement *node, Statement *ast) {
        if (ProcessDependencySymbolIfNeeded(ast) < 0) {
            return -1;
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
    
    int ProcessDependencySymbolIfNeeded(Statement *ast) {
        if (ast->IsTemplate()) {
            return 0; // Ignore template
        }
        auto [owns, sym] = ast->Owns(true/*force*/);
        auto pkg = ast->Pack(true/*force*/);
        if (auto pkg_scope = FindPackageScopeOrNull(pkg); pkg_scope && pkg_scope->HasNotTracked(ast)) {
            if (owns && owns->IsFileUnit()) {
                auto current_pkg_scope = location_->NearlyPackageScope();
                auto current_file_scope = location_->NearlyFileUnitScope();
                
                if (owns != current_file_scope->file_unit()) {
                    printd("nested reduce: %s", down_cast<FileUnit>(owns)->file_name()->data());
                    PackageScope *external_scope = nullptr;
                    FileUnitScope *scope = nullptr;
                    if (pkg != current_pkg_scope->pkg()) {
                        external_scope = EnsurePackageScope(pkg);
                        external_scope->Enter();
                        scope = DCHECK_NOTNULL(external_scope->FindFileUnitScopeOrNull(owns));
                    } else {
                        scope = DCHECK_NOTNULL(current_pkg_scope->FindFileUnitScopeOrNull(owns));
                    }
                    assert(scope != current_file_scope);
                    scope->Enter();
                    auto rs = Reduce(sym);
                    pkg_scope->Track(sym);
                    scope->Exit();
                    if (external_scope) { external_scope->Exit(); }
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
        return 0;
    }
    
    int VisitInterfaceDefinition(InterfaceDefinition *node) override {
        if (!node->generic_params().empty()) {
            return Return(Unit());
        }
        
        for (auto method : node->methods()) {
            if (Reduce(method) < 0) {
                return -1;
            }
        }
        return Return(Unit());
    }
    

    int VisitAssignment(Assignment *node) override { UNREACHABLE(); }
    int VisitStructDefinition(StructDefinition *node) override { UNREACHABLE(); }
    
    int VisitAnnotationDefinition(AnnotationDefinition *node) override { UNREACHABLE(); }
    
    
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
        return type->Link(std::bind(&TypeReducingVisitor::FindType, this, std::placeholders::_1,
                                    std::placeholders::_2));
    }
    
    Type *FindType(const Symbol *name, Type *owns) {
        Statement *symbol = nullptr;
        bool should_inst = false;
        auto file_scope = DCHECK_NOTNULL(location_->NearlyFileUnitScope());
        if (!owns->generic_args().empty()) {
            auto full_name = MakeFullInstantiatingName(name->name()->ToSlice(), owns->generic_args_size(),
                                                       &owns->generic_args()[0]);
            if (name->prefix_name()) {
                symbol = file_scope->FindExportSymbol(name->prefix_name()->ToSlice(), full_name);
            } else {
                symbol = file_scope->FindLocalSymbol(full_name);
            }
            should_inst = symbol == nullptr;
        }
        if (!symbol) {
            if (name->prefix_name()) {
                symbol = file_scope->FindExportSymbol(name->prefix_name()->ToSlice(), name->name()->ToSlice());
            } else {
                symbol = file_scope->FindLocalSymbol(name->name()->ToSlice());
            }
        }
        if (!symbol) {
            Feedback()->Printf(name->source_position(), "symbol: %s not found!", name->ToString().c_str());
            return nullptr;
        }
        if (should_inst) {
            InstantiatingResolver resolver(this);
            auto rs = GenericsInstantiating::Instantiate(nullptr, symbol, arena_, error_feedback_, &resolver,
                                                         owns->generic_args_size(),
                                                         const_cast<Type **>(&owns->generic_args()[0]),
                                                         &symbol);
            if (rs.fail()) {
                return nullptr;
            }
        }
        
        
        switch (symbol->kind()) {
            case Node::kClassDefinition:
                return new (arena_) ClassType(arena_, symbol->AsClassDefinition(), name->source_position());
            case Node::kStructDefinition:
                return new (arena_) StructType(arena_, symbol->AsStructDefinition(), name->source_position());
            case Node::kInterfaceDefinition:
                return new (arena_) InterfaceType(arena_, symbol->AsInterfaceDefinition(), name->source_position());
            case Node::kObjectDeclaration:
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
        if (auto id = expr->AsIdentifier()) {
            return std::make_tuple(default_prefix, id->name()->ToSlice());
        } else if (auto dot = expr->AsDot()) {
            //assert(expr->IsDot());
            auto prefix = DCHECK_NOTNULL(dot->primary()->AsIdentifier());
            auto name = dot->field();
            return std::make_tuple(prefix->name()->ToSlice(), name->ToSlice());
        } else if (auto inst = expr->AsInstantiation()) {
            if (auto id = inst->primary()->AsIdentifier()) {
                return std::make_tuple(default_prefix, id->name()->ToSlice());
            } else if (auto dot = inst->primary()->AsDot()) {
                auto prefix = DCHECK_NOTNULL(dot->primary()->AsIdentifier());
                auto name = dot->field();
                return std::make_tuple(prefix->name()->ToSlice(), name->ToSlice());
            } else {
                UNREACHABLE();
            }
        } else {
            UNREACHABLE();
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
    
    void NewPackageScopeIfNeeded(Package *pkg) {
        if (auto iter = package_scopes_.find(pkg); iter != package_scopes_.end()) {
            return;
        }
        package_scopes_[pkg] = new PackageScope(&location_, pkg, &global_symbols_);
    }
    
    PackageScope *EnsurePackageScope(Package *pkg) const {
        auto iter = package_scopes_.find(pkg);
        assert(iter != package_scopes_.end());
        return iter->second;
    }
    
    PackageScope *FindPackageScopeOrNull(Package *pkg) const {
        auto iter = package_scopes_.find(pkg);
        if (iter == package_scopes_.end()) {
            return nullptr;
        }
        return iter->second;
    }
    
    bool MaybePackageName(Identifier *id) {
        auto [ast, ns] = location_->FindSymbol(id->name()->ToSlice());
        return !ast;
    }
    
    static std::string MakeFullInstantiatingName(std::string_view name, int argc, Type *const *argv) {
        std::string buf(name);
        buf.append("<");
        for (size_t i = 0; i < argc; i++) {
            if (i > 0) {
                buf.append(",");
            }
            buf.append(argv[i]->ToString()); // FIXME: use full type name
        }
        buf.append(">");
        return buf;
    }
    
    base::Arena *const arena_;
    SyntaxFeedback *const error_feedback_;
    NamespaceScope *location_ = nullptr;
    Package *entry_;
    std::set<Package *> track_;
    base::Status status_;
    std::unordered_map<std::string_view, GlobalSymbol> global_symbols_;
    std::map<Package *, PackageScope *> package_scopes_;
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
