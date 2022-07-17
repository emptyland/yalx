#include "compiler/generics-instantiating.h"
#include "compiler/syntax-feedback.h"
#include "compiler/scope.h"
#include "compiler/ast.h"
#include "compiler/constants.h"
#include "compiler/compiler.h"
#include <stack>

namespace yalx {

namespace cpl {

class SymbolDepsScope {
public:
    SymbolDepsScope(SymbolDepsScope **location, PackageScope *pkg_scope)
    : location_(location)
    , prev_(nullptr)
    , pkg_(DCHECK_NOTNULL(pkg_scope)->pkg()) {
        Enter();
    }
    
    ~SymbolDepsScope() { Exit(); }
    
    DEF_PTR_GETTER(SymbolDepsScope, prev);
    DEF_PTR_GETTER(Package, pkg);
    
    void AddForward(base::Arena *arena, std::string_view name, Statement *ast) {
        AddForward(pkg_->FindOrInsertDeps(arena, name, ast));
    }
    
    void AddForward(SymbolDepsNode *node) { forwards_.push_back(node); }
    
    void AddBackward(base::Arena *arena, std::string_view name, Statement *ast) {
        AddBackward(pkg_->FindOrInsertDeps(arena, name, ast));
    }
    
    void AddBackward(SymbolDepsNode *node) {
        for (auto fw : forwards_) { fw->AddBackward(node); }
    }

private:
    void Enter() {
        DCHECK(*location_ != this);
        prev_ = *location_;
        *location_ = this;
    }
    void Exit() {
        DCHECK(*location_ == this);
        DCHECK(*location_ != prev_);
        *location_ = prev_;
    }
    
    SymbolDepsScope **location_;
    SymbolDepsScope *prev_;
    Package *pkg_;
    std::vector<SymbolDepsNode *> forwards_;
};

class PrimitiveTypesProperties final {
public:
    PrimitiveTypesProperties(base::Arena *arena)
        : arena_(DCHECK_NOTNULL(arena))
        , props_(arena) {}
    
    static uintptr_t Uniquely() {
        static int dummy;
        return reinterpret_cast<uintptr_t>(&dummy);
    }
    
    void Init();
    
    Statement *FindMemberOrNull(const Type *ty, std::string_view name) const;
    
    Type *kI32Ty = nullptr;
    Type *kStringTy = nullptr;
    Type *kAnyTy = nullptr;
    
private:
    void Put(std::string_view owns, std::string_view name, Type *ty) {
        auto id = String::New(arena_, name);
        auto field = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal, id, ty,
                                                      SourcePosition::Unknown());
        props_[MakeFullName(owns, name)] = field;
    }
    
    void Put(std::string_view owns, std::string_view name, std::initializer_list<Type *> params,
             std::initializer_list<Type *> returning_ty) {
        auto proto = new (arena_) FunctionPrototype(arena_, false/*vargs*/, SourcePosition::Unknown());
        for (auto ty : params) {
            proto->mutable_params()->push_back(ty);
        }
        for (auto ty : returning_ty) {
            proto->mutable_return_types()->push_back(ty);
        }
        auto id = String::New(arena_, name);
        auto fun = new (arena_) FunctionDeclaration(arena_, FunctionDeclaration::kNative, id, proto, false/*is_reduce*/,
                                                    SourcePosition::Unknown());
        props_[MakeFullName(owns, name)] = fun;
    }
    
    std::string_view MakeFullName(std::string_view owns, std::string_view name) {
        auto full_name = static_cast<char *>(arena_->Allocate(owns.size() + name.size() + 3));
        strncpy(full_name, owns.data(), owns.size() + 1);
        strncat(full_name, "::", 3);
        strncat(full_name, name.data(), name.size() + 1);
        return std::string_view(full_name);
    }
    
    base::Arena *const arena_;
    base::ArenaMap<std::string_view, Statement *> props_;
}; // class PrimitiveTypesProperties

void PrimitiveTypesProperties::Init() {
    kI32Ty = new (arena_) Type(arena_, Type::kType_i32, SourcePosition::Unknown());
    kStringTy = new (arena_) Type(arena_, Type::kType_string, SourcePosition::Unknown());
    kAnyTy = new (arena_) Type(arena_, Type::kType_any, SourcePosition::Unknown());
    
    Put("Array", "size", kI32Ty);
    
    Put("MultiDimsArray", "size", kI32Ty);
    Put("MultiDimsArray", "rank", kI32Ty);
    Put("MultiDimsArray", "getLength", {kI32Ty}, {kI32Ty}); // fun getLength(i32): i32
    
    Put("u8", "toString", {}, {kStringTy});
    Put("i8", "toString", {}, {kStringTy});
    Put("u16", "toString", {}, {kStringTy});
    Put("i16", "toString", {}, {kStringTy});
    Put("u32", "toString", {}, {kStringTy});
    Put("i32", "toString", {}, {kStringTy});
    Put("u64", "toString", {}, {kStringTy});
    Put("i64", "toString", {}, {kStringTy});
    Put("f32", "toString", {}, {kStringTy});
    Put("f64", "toString", {}, {kStringTy});
    
    Put("String", "size", kI32Ty);
}

Statement *PrimitiveTypesProperties::FindMemberOrNull(const Type *ty, std::string_view name) const {
    char buf[512] = {0};
    switch (ty->category()) {
        case Type::kArray: {
            auto ar = ty->AsArrayType();
            if (ar->dimension_count() == 1) {
                strncpy(buf, "Array::", arraysize(buf));
            } else {
                DCHECK(ar->dimension_count() > 1);
                strncpy(buf, "MultiDimsArray::", arraysize(buf));
            }
            strncat(buf, name.data(), name.size());
            if (auto iter = props_.find(buf); iter != props_.end()) {
                return iter->second;
            }
        } return nullptr;
        case Type::kPrimary:
            strncpy(buf, ty->ToString().c_str(), arraysize(buf));
            strncat(buf, "::", arraysize(buf));
            strncat(buf, name.data(), arraysize(buf));
            if (auto iter = props_.find(buf); iter != props_.end()) {
                return iter->second;
            }
            break;
        default:
            return nullptr;
    }
    
    UNREACHABLE();
    return nullptr;
}

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
            DCHECK(owns->IsFileUnit());
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
        //printd("prepare package: %s", pkg->name()->data());
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
            
            for (auto stmt : file->struct_defs()) {
                auto symbol = FindOrInsertGlobal(node, stmt->name()->ToSlice(), stmt);
                if (symbol.IsFound()) {
                    Feedback()->Printf(stmt->source_position(), "Duplicated symbol: %s", stmt->name()->data());
                    return;
                }
            }
            
            for (auto stmt : file->enum_defs()) {
                auto symbol = FindOrInsertGlobal(node, stmt->name()->ToSlice(), stmt);
                if (symbol.IsFound()) {
                    Feedback()->Printf(stmt->source_position(), "Duplicated symbol: %s", stmt->name()->data());
                    return;
                }
            }
            
            for (auto stmt : file->objects()) {
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
                        auto any = FindGlobal(kLangPackageFullName, kAnyClassName); // Any class
                        DCHECK(any.IsFound());
                        clazz->set_base_of(DCHECK_NOTNULL(any.ast->AsClassDefinition()));
                    }
                    continue;
                }
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
        //printd("%s", node->full_path()->data());
        if (Track(node)) {
            return 0; // Skip if has processed
        }
        NamespaceScope::Keeper<PackageScope> keeper(EnsurePackageScope(node));
        error_feedback_->set_package_name(node->name()->ToString());
        
        //printd("process package: %s", node->name()->data());
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
            if (auto nrets = Reduce(ast); nrets < 0) {
                return -1;
            }
        }
        return Returning(Unit());
    }
    
    int VisitBlock(Block *node) override {
        BlockScope scope(&location_, kPlainBlock, node);
        std::vector<Type *> types = { Unit() };
        for (auto ast : node->statements()) {
            types.clear();
            if (Reduce(ast, &types) < 0) {
                return -1;
            }
        }
        return Returning(types);
    }
    
    int VisitList(List *node) override {
        std::vector<Type *> types;
        for (auto ast : node->expressions()) {
            if (Reduce(ast, &types) < 0) {
                return -1;
            }
        }
        DCHECK(!types.empty());
        return Returning(types);
    }
    
    int VisitVariableDeclaration(VariableDeclaration *node) override {
        std::unique_ptr<SymbolDepsScope> deps_scope;
        if (!node->owns()) {
            for (auto var : node->variables()) {
                if (Identifier::IsPlaceholder(var->identifier())) {
                    continue;
                }
                if (auto dup = location_->FindOrInsertSymbol(var->identifier()->ToSlice(), var)) {
                    Feedback()->Printf(var->source_position(), "Duplicated symbol: %s", var->identifier()->data());
                    return -1;
                }
            }
        } else if (node->owns()->IsFileUnit()) {
            //auto pkg = DCHECK_NOTNULL(location_->NearlyPackageScope())->pkg();
            deps_scope.reset(new SymbolDepsScope(&deps_, location_->NearlyPackageScope()));
            for (auto var : node->variables()) {
                if (Identifier::IsPlaceholder(var->identifier())) {
                    continue;
                }
                deps_scope->AddForward(arena_, var->identifier()->ToSlice(), node);
                if (var->type()) {
                    auto linked = LinkType(var->type());
                    if (!linked) {
                        return -1;
                    }
                    var->set_type(linked);
                }
            }
        }
        
        std::vector<Type *> types;
        for (auto expr : node->initilaizers()) {
            if (Reduce(expr, &types) < 0) {
                return -1;
            }
        }

        if (types.size() != node->variables_size()) {
            Feedback()->Printf(node->source_position(),
                               "Different declaration numbers and initilizer numbers, %zd vs %zd",
                               node->variables_size(), types.size());
            return -1;
        }
        
        for (size_t i = 0; i < node->variables_size(); i++) {
            auto var = node->variable(i);
            if (Identifier::IsPlaceholder(var->identifier())) {
                continue;
            }
            if (!var->type()) {
                var->set_type(types[i]);
            } else {
                if (!AcceptableOrLink(var->mutable_type(), types[i])) {
                    Feedback()->Printf(var->source_position(), "LVal is not acceptable from RVal");
                    return -1;
                }
            }
        }
        return Returning(Unit());
    }
    
    int VisitObjectDeclaration(ObjectDeclaration *node) override {
        if (node->dummy() && node->dummy()->IsClassType()) {
            return Returning(Unit());
        }
        
        std::string buf(node->name()->ToString().append(kObjectShadowClassPostfix));
        auto shadow_class = new (arena_) ClassDefinition(arena_, String::New(arena_, buf.data(), buf.size()),
                                                         node->source_position());
        for (auto field : node->fields()) {
            shadow_class->mutable_fields()->push_back(FieldOfStructure{
                false,
                0,
                field,
            });
        }
        for (auto method : node->methods()) {
            shadow_class->mutable_methods()->push_back(method);
        }
        shadow_class->set_package(node->package());
        shadow_class->set_owns(node->owns());
        
        auto type = new (arena_) ClassType(arena_, shadow_class, node->source_position());
        node->set_dummy(type);
        
        DCHECK_NOTNULL(shadow_class->owns()->AsFileUnit())->Add(shadow_class);
        FindOrInsertGlobal(shadow_class->package(), shadow_class->name()->ToSlice(), shadow_class);
        
        SymbolDepsScope deps_scope(&deps_, location_->NearlyPackageScope());
        deps_scope.AddForward(arena_, shadow_class->name()->ToSlice(), shadow_class);
        if (Reduce(shadow_class) < 0) {
            return -1;
        }

        return Returning(Unit());
    }
    
    int VisitClassDefinition(ClassDefinition *node) override {
        //printd("%s", node->FullName().c_str());
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        SymbolDepsScope deps_scope(&deps_, location_->NearlyPackageScope());
        deps_scope.AddForward(arena_, node->name()->ToSlice(), node);
        
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
            auto base_of = symbol->AsClassDefinition();
            node->set_base_of(base_of);
        }
        if (node->base_of() && ProcessDependencySymbolIfNeeded(node->base_of()) < 0) {
            return -1;
        }
        if (node->base_of() && node->base_of()->package() == node->package()) {
            deps_scope.AddBackward(arena_, node->base_of()->name()->ToSlice(), node->base_of());
        }
        
        //auto classes_count = 0;
        for (auto i = 0; i < node->concepts_size(); i++) {
            auto concept = LinkType(node->concept(i));
            if (!concept) {
                return -1;
            }

            if (!concept->IsInterfaceType()) {
                Feedback()->Printf(concept->source_position(), "Concept must be a interface");
                return -1;
            }
            if (ProcessDependencySymbolIfNeeded(concept->AsInterfaceType()->definition()) < 0) {
                return -1;
            }
            *node->mutable_concept(i) = concept;
        }
        
        // Default base class is Any;
        if (!node->base_of()) {
            auto any_class = FindGlobal(kLangPackageFullName, kAnyClassName);
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
        scope.InstallAncestorsSymbols();
        scope.InstallConcepts();
        auto constructor = GeneratePrimaryConstructor(node, node->base_of(), node->super_calling());
        constructor->set_owns(node);
        node->set_primary_constructor(constructor);

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
            if (LinkType(method->prototype()) == nullptr) {
                return -1;
            }

            if (!method->body()) {
                method->prototype()->set_signature(MakePrototypeSignature(method->prototype()));
            } else if (Reduce(method) < 0) {
                return -1;
            }
            
            if (method->decoration() == FunctionDeclaration::kOverride) {
                // check override
                auto target = scope.ImplementMethodOnce(method->name()->ToSlice(), method->prototype()->signature());
                if (target == DataDefinitionScope::kNotFound) {
                    Feedback()->Printf(method->source_position(), "Invalid `override' decoration: %s%s",
                                       method->name()->data(), method->prototype()->signature()->data());
                    return -1;
                }
            }
        }
        
        if (scope.UnimplementMethods([this, node](auto ift, auto method) {
            Feedback()->Printf(node->source_position(), "Unimplement method: %s::%s%s",
                               ift->name()->data(), method->name()->data(), method->prototype()->signature()->data());
        }) > 0) {
            return -1;
        }
        
        if (!constructor || Reduce(constructor) < 0) {
            return -1;
        }
        return Returning(Unit());
    }
    
    int VisitStructDefinition(StructDefinition *node) override {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        SymbolDepsScope deps_scope(&deps_, location_->NearlyPackageScope());
        deps_scope.AddForward(arena_, node->name()->ToSlice(), node);

        if (node->super_calling()) {
            auto super_call = node->super_calling();
            Statement *symbol = nullptr;
            switch (super_call->callee()->kind()) {
                case Node::kIdentifier:
                    symbol = ResolveIdSymbol("base struct", super_call->callee()->AsIdentifier(), Node::kStructDefinition);
                    break;
                case Node::kDot:
                    symbol = ResolveDotSymbol("base struct", super_call->callee()->AsDot(), Node::kStructDefinition);
                    break;
                case Node::kInstantiation:
                    symbol = Instantiate("base struct", super_call->callee()->AsInstantiation(), Node::kStructDefinition);
                    break;
                default:
                    break;
            }
            if (!symbol) {
                return -1;
            }
            node->set_base_of(symbol->AsStructDefinition());
        }
        if (node->base_of() && ProcessDependencySymbolIfNeeded(node->base_of()) < 0) {
            return -1;
        }
        if (node->base_of() && node->base_of()->package() == node->package()) {
            deps_scope.AddBackward(arena_, node->base_of()->name()->ToSlice(), node->base_of());
        }
        
        DataDefinitionScope scope(&location_, node);
        scope.InstallAncestorsSymbols();
        auto constructor = GeneratePrimaryConstructor(node, node->base_of(), node->super_calling());
        if (!constructor || Reduce(constructor) < 0) {
            return -1;
        }
        constructor->set_owns(node);
        node->set_primary_constructor(constructor);
        
        // Into struct scope:
        for (int i = 0; i < node->fields_size(); i++) {
            auto field = node->field(i);
            if (field.in_constructor) {
                auto name = field.declaration->Identifier();
                auto dup = scope.FindOrInsertSymbol(name->ToSlice(), field.declaration);
                if (dup) {
                    Feedback()->Printf(field.declaration->source_position(), "duplicated struct field: `%s'",
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
            if (LinkType(method->prototype()) == nullptr) {
                return -1;
            }
            if (!method->body()) {
                method->prototype()->set_signature(MakePrototypeSignature(method->prototype()));
            } else if (Reduce(method) < 0) {
                return -1;
            }
            
            if (method->decoration() == FunctionDeclaration::kOverride) {
                // check override
                auto target = scope.ImplementMethodOnce(method->name()->ToSlice(), method->prototype()->signature());
                if (target == DataDefinitionScope::kNotFound) {
                    Feedback()->Printf(method->source_position(), "Invalid `override' decoration: %s%s",
                                       method->name()->data(), method->prototype()->signature()->data());
                    return -1;
                }
            }
        }
        return Returning(Unit());
    }
    
    int VisitEnumDefinition(EnumDefinition *node) override {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        SymbolDepsScope deps_scope(&deps_, location_->NearlyPackageScope());
        deps_scope.AddForward(arena_, node->name()->ToSlice(), node);
        
        base::ArenaUnorderedMap<std::string_view, VariableDeclaration *> values(arena_);
        for (const auto &field : node->fields()) {
            auto decl = field.declaration;
            if (auto iter = values.find(decl->name()->ToSlice()); iter != values.end()) {
                Feedback()->Printf(decl->source_position(), "Duplicated value named: `%s'", decl->name()->data());
                return -1;
            }
            values[decl->name()->ToSlice()] = decl;
            
            for (int i = 0; i < decl->variables_size(); i++) {
                auto item = decl->variable(i);
                auto ty = LinkType(item->Type());
                if (!ty) {
                    return -1;
                }
                item->set_type(ty);
            }
        }

        DataDefinitionScope scope(&location_, node);
        for (auto method : node->methods()) {
            if (LinkType(method->prototype()) == nullptr) {
                return -1;
            }
            if (!method->body()) {
                method->prototype()->set_signature(MakePrototypeSignature(method->prototype()));
            } else if (Reduce(method) < 0) {
                return -1;
            }
            
            // TODO:
        }
        return Returning(Unit());
    }
    
    FunctionDeclaration *GeneratePrimaryConstructor(IncompletableDefinition *node,
                                                    IncompletableDefinition *base_of,
                                                    Calling *super_calling) {
        auto prototype = new (arena_) FunctionPrototype(arena_, false/*vargs*/, node->source_position());

        for (auto param : node->parameters()) {
            VariableDeclaration::Item *var = nullptr;
            if (param.field_declaration) {
                var = down_cast<VariableDeclaration::Item>(node->field(param.as_field).declaration->AtItem(0));
            } else {
                var = param.as_parameter;
            }
            prototype->mutable_params()->push_back(var);
        }
        
        auto buf = node->name()->ToString().append(kPrimaryConstructorPostfix);
        auto name = String::New(arena_, buf.data(), buf.size());
        auto fun = new (arena_) FunctionDeclaration(arena_, FunctionDeclaration::kDefault, name, prototype,
                                                    false/*is_reduct*/, node->source_position());
        auto body = new (arena_) Block(arena_, node->source_position());
        fun->set_body(body);
        
        auto this_id = new (arena_) Identifier(String::New(arena_, "this"), node->source_position());
        if (base_of) {
            //printd("base_of: %s", base_of->name()->data());
            DCHECK(base_of->primary_constructor());
            auto super_id = new (arena_) Identifier(base_of->primary_constructor()->name(), node->source_position());
            if (!super_calling) {
                auto calling = new (arena_) Calling(arena_, super_id, node->source_position());
                body->mutable_statements()->push_back(calling);
            } else {
                auto calling = new (arena_) Calling(arena_, super_id, node->source_position());
                calling->mutable_args()->assign(super_calling->args().begin(), super_calling->args().end());
                body->mutable_statements()->push_back(calling);
            }
        }

        for (auto param : node->parameters()) {
            if (param.field_declaration) {
                auto var = node->field(param.as_field).declaration;
                auto dot = new (arena_) Dot(this_id, var->Identifier(), var->source_position());
                auto ass = new (arena_) Assignment(arena_, var->source_position());
                ass->set_initial(true);
                ass->mutable_lvals()->push_back(dot);
                ass->mutable_rvals()->push_back(new (arena_) Identifier(var->Identifier(), var->source_position()));
                body->mutable_statements()->push_back(ass);
            }
        }
        for (auto field : node->fields()) {
            if (field.in_constructor) {
                continue;
            }
            
            auto ass = new (arena_) Assignment(arena_, field.declaration->source_position());
            ass->set_initial(true);
            for (auto i = 0; i < field.declaration->ItemSize(); i++) {
                auto var = field.declaration->AtItem(i);
                auto dot = new (arena_) Dot(this_id, var->Identifier(), var->source_position());
                ass->mutable_lvals()->push_back(dot);
            }
            for (auto expr : field.declaration->initilaizers()) {
                ass->mutable_rvals()->push_back(expr);
            }
            body->mutable_statements()->push_back(ass);
        }
        
        auto ret = new (arena_) class Return(arena_, node->source_position());
        //ret->mutable_returnning_vals()->push_back(unit_);
        body->mutable_statements()->push_back(ret);
        return fun;
    }

    int VisitFunctionDeclaration(FunctionDeclaration *node) override {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }

        FunctionScope scope(&location_, node->prototype(), node->is_reduce());
        std::unique_ptr<SymbolDepsScope> deps_scope;
        if (node->owns() && node->owns()->IsFileUnit()) {
            deps_scope.reset(new SymbolDepsScope(&deps_, location_->NearlyPackageScope()));
            deps_scope->AddForward(arena_, node->name()->ToSlice(), node);
        }
        
        if (!LinkType(node->prototype())) {
            return -1;
        }

        if (node->IsNative() || node->IsAbstract()) {
            if (node->prototype()->return_types().empty()) {
                node->prototype()->mutable_return_types()->push_back(Unit());
            }
            node->prototype()->set_signature(MakePrototypeSignature(node->prototype()));
            return Returning(Unit()); // Ignore nobody funs
        }
        
        // Install `this' variable if needed
        if (auto data_scope = location_->NearlyDataDefinitionScope()) {
            scope.FindOrInsertSymbol("this", data_scope->ThisStub(arena_));
            if (auto duplicated = location_->FindOrInsertSymbol(node->name()->ToSlice(), node)) {
                Feedback()->Printf(node->source_position(), "Duplicated function name: %s", node->name()->data());
                return -1;
            }
        }
        
        auto prototype = node->prototype();
        if (prototype->vargs()) {
            auto stub = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal,
                                                         String::New(arena_, kArgsName),
                                                         Array(Any()),
                                                         prototype->source_position());
            scope.FindOrInsertSymbol(stub->AtItem(0)->Identifier()->ToSlice(), stub->AtItem(0));
        }
        
        for (auto item : prototype->params()) {
            auto param = static_cast<VariableDeclaration::Item *>(item);
            if (Identifier::IsPlaceholder(param->identifier())) {
                continue;
            }
            if (auto dup = scope.FindOrInsertSymbol(param->identifier()->ToSlice(), param)) {
                Feedback()->Printf(param->source_position(), "Duplicated fun parameter: %s, prev one: %s",
                                   param->identifier()->data(), dup->source_position().ToString().c_str());
                return -1;
            }
        }

        std::vector<Type *> receiver;
        int nrets = Reduce(node->body(), &receiver);
        if (nrets < 0) {
            return -1;
        }
        if (!node->is_reduce()) {
            // Install signature at last step
            if (node->prototype()->return_types().empty()) {
                node->prototype()->mutable_return_types()->push_back(Unit());
            }
            node->prototype()->set_signature(MakePrototypeSignature(node->prototype()));
            return Returning(Unit());
        }

        if (prototype->return_types_size() > 0) {
            if (prototype->return_types_size() != nrets) {
                Feedback()->Printf(node->source_position(), "Unexpected return val numbers, %d, %zd", nrets,
                                   prototype->return_types_size());
                return -1;
            }
            
            for (auto i = 0; i < prototype->return_types_size(); i++) {
                bool unlinked = false;
                if (!receiver[i]->Acceptable(prototype->return_type(i), &unlinked)) {
                    Feedback()->Printf(node->source_position(), "Return type does not accept, %s <= %s",
                                       prototype->return_type(i)->ToString().c_str(),
                                       receiver[i]->ToString().c_str());
                    return -1;
                }
            }
        } else {
            DCHECK(prototype->return_types_size() == 0);
            for (auto ty : receiver) {
                prototype->mutable_return_types()->push_back(ty);
            }
        }
        
        // Install signature at last step
        node->prototype()->set_signature(MakePrototypeSignature(node->prototype()));
        return Returning(Unit());
    }
    
    int VisitLambdaLiteral(LambdaLiteral *node) override {
        FunctionScope scope(&location_, node->prototype(), true/*fun_is_reducing*/);
        
        if (!LinkType(node->prototype())) {
            return -1;
        }

        auto prototype = node->prototype();
        {
            auto stub = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal,
                                                         String::New(arena_, kCalleeName),
                                                         prototype,
                                                         prototype->source_position());
            scope.FindOrInsertSymbol(stub->AtItem(0)->Identifier()->ToSlice(), stub->AtItem(0));
        }
        
        if (prototype->vargs()) {
            auto stub = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal,
                                                         String::New(arena_, kArgsName),
                                                         Array(Any()),
                                                         prototype->source_position());
            scope.FindOrInsertSymbol(stub->AtItem(0)->Identifier()->ToSlice(), stub->AtItem(0));
        }

        for (auto item : prototype->params()) {
            auto param = static_cast<VariableDeclaration::Item *>(item);
            if (Identifier::IsPlaceholder(param->identifier())) {
                continue;
            }
            if (auto dup = scope.FindOrInsertSymbol(param->identifier()->ToSlice(), param)) {
                Feedback()->Printf(param->source_position(), "Duplicated fun parameter: %s, prev one: %s",
                                   param->identifier()->data(), dup->source_position().ToString().c_str());
                return -1;
            }
        }
        
        std::vector<Type *> receiver;
        int nrets = Reduce(node->body(), &receiver);
        if (nrets < 0) {
            return -1;
        }
        
        if (prototype->return_types_size() > 0) {
            if (prototype->return_types_size() != nrets) {
                Feedback()->Printf(node->source_position(), "Unexpected return val numbers, %d, %zd", nrets,
                                   prototype->return_types_size());
                return -1;
            }
            
            for (auto i = 0; i < prototype->return_types_size(); i++) {
                bool unlinked = false;
                if (!receiver[i]->Acceptable(prototype->return_type(i), &unlinked)) {
                    Feedback()->Printf(node->source_position(), "Return type not accepted, %s <= %s",
                                       prototype->return_type(i)->ToString().c_str(),
                                       receiver[i]->ToString().c_str());
                    return -1;
                }
            }
        } else {
            DCHECK(prototype->return_types_size() == 0);
            for (auto ty : receiver) {
                prototype->mutable_return_types()->push_back(ty);
            }
        }
        if (prototype->return_types().empty()) {
            prototype->mutable_return_types()->push_back(Unit());
        }
        
        // Install signature at last step
        node->prototype()->set_signature(MakePrototypeSignature(node->prototype()));
        return Returning(prototype);
    }
    
    int VisitIdentifier(Identifier *node) override {
        if (node->IsPlaceholder()) {
            Feedback()->Printf(node->source_position(), "Do not use placeholder(`_')");
            return -1;
        }
        auto [ast, ns] = location_->FindSymbol(node->name()->ToSlice());
        if (!ast) {
            Feedback()->Printf(node->source_position(), "Symbol: `%s' not found", node->name()->data());
            return -1;
        }
        return ReduceDependencySymbolIfNeeded(node->name()->data(), node, ast);
    }
    
    int VisitDot(Dot *node) override {
        if (auto id = node->primary()->AsIdentifier(); id && MaybePackageName(id)) {
            auto file_scope = location_->NearlyFileUnitScope();
            auto symbol = file_scope->FindExportSymbol(id->name()->ToSlice(), node->field()->ToSlice());
            if (!symbol) {
                Feedback()->Printf(node->source_position(), "Symbol: %s.%s not found", id->name()->data(),
                                   node->field()->data());
                return -1;
            }
            return ReduceDependencySymbolIfNeeded(id->name()->data(), node, symbol);
        }
        std::vector<Type *> types;
        if (Reduce(node->primary(), &types) < 0) {
            return -1;
        }
        
        if (types.size() > 1) {
            Feedback()->Printf(node->source_position(), "Too many values for field: %s", node->field()->data());
            return -1;
        }
        
        auto def = GetTypeSpecifiedDefinition(types[0]);
        if (!def) {
            auto ast = props_->FindMemberOrNull(types[0], node->field()->ToSlice());
            if (!ast) {
                Feedback()->Printf(node->source_position(), "Field: `%s' not found", node->field()->data());
                return -1;
            }
            // TODO:
            // a.size
            // a.rank
            // a.getLength(0)
            if (auto dest = ast->AsVariableDeclaration()) {
                return Returning(dest->Type());
            } else if (auto dest = ast->AsFunctionDeclaration()) {
                return Returning(dest->prototype());
            } else {
                UNREACHABLE();
            }
        }
        if (ProcessDependencySymbolIfNeeded(def) < 0) {
            return -1;
        }
        
        Statement *field = nullptr;
        const IncompletableDefinition *level = nullptr;
        if (auto clazz = def->AsClassDefinition()) {
            auto [ast, ns] = clazz->FindSymbolWithOwns(node->field()->ToSlice());
            field = ast;
            level = ns;
        } else if (auto clazz = def->AsStructDefinition()) {
            auto [ast, ns] = clazz->FindSymbolWithOwns(node->field()->ToSlice());
            field = ast;
            level = ns;
        } else if (auto clazz = def->AsInterfaceDefinition()) {
            field = clazz->FindSymbolOrNull(node->field()->ToSlice());
        } else {
            UNREACHABLE();
        }
        
        if (!field) {
            Feedback()->Printf(node->source_position(), "Field `%s' not found", node->field()->data());
            return -1;
        }
        if (level && !CheckAccessable(down_cast<IncompletableDefinition>(def), field, level, node->source_position())) {
            return -1;
        }
        
        switch (field->kind()) {
            case Node::kVariableDeclaration:
                return Returning(field->AsVariableDeclaration()->Type());
            case Node::kFunctionDeclaration:
                return Returning(field->AsFunctionDeclaration()->prototype());
            default: {
                if (auto var = down_cast<VariableDeclaration::Item>(field)) {
                    return Returning(var->type());
                }
                UNREACHABLE();
                return -1;
            }
        }
    }
    
    int VisitResolving(Resolving *node) override {
        Statement *owns = nullptr;
        if (!ReduceResolving(node, &owns)) {
            return -1;
        }
        return Returning(new (arena_) EnumType(arena_, DCHECK_NOTNULL(owns->AsEnumDefinition()),
                                               node->source_position()));
    }
    
    Statement *ReduceResolving(Resolving *node, Statement **owns = nullptr) {
        Statement *ast = nullptr;
        switch (node->primary()->kind()) {
            case Node::kInstantiation:
                ast = Instantiate("resolving", node->primary()->AsInstantiation(), Node::kMaxKinds);
                break;
                
            case Node::kIdentifier: {
                auto id = node->primary()->AsIdentifier();
                auto [found, ns] = location_->FindSymbol(id->name()->ToSlice());
                ast = found;
            } break;
                
            case Node::kDot: {
                auto dot = node->primary()->AsDot();
                ast = ResolveDotSymbol("Enum definition", dot, Node::kEnumDefinition);
                if (!ast) {
                    return nullptr;
                }
            } break;
                
            default:
                goto invalid;
        }
        
        if (auto def = ast->AsEnumDefinition()) {
            auto val = def->FindSymbolOrNull(node->field()->ToSlice());
            if (val) {
                if (owns) { *owns = def; }
                return val;
            }
        }
        
    invalid:
        Feedback()->Printf(node->source_position(), "Invalid `::' token");
        return nullptr;
    }

    int VisitInterfaceDefinition(InterfaceDefinition *node) override {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        
        SymbolDepsScope deps_scope(&deps_, location_->NearlyPackageScope());
        deps_scope.AddForward(arena_, node->name()->ToSlice(), node);

        std::map<std::string_view, std::string_view> method_names;
        for (auto method : node->methods()) {
            if (LinkType(method->prototype()) == nullptr) {
                return -1;
            }
            method->prototype()->set_signature(MakePrototypeSignature(method->prototype()));
            method->set_owns(node);
            
            auto iter = method_names.find(method->name()->ToSlice());
            if (iter != method_names.end()) {
                Feedback()->Printf(method->source_position(), "Duplicated interface method name: %s",
                                   method->name()->data());
                return -1;
            }
            method_names[method->name()->ToSlice()] = method->prototype()->signature()->ToSlice();
        }
        return Returning(Unit());
    }
    
    int VisitCalling(Calling *node) override {
        Statement *ast = nullptr;
        bool is_others_expr = false;
        switch (node->callee()->kind()) {
            case Node::kIdentifier: {
                auto [sym, ns] = location_->FindSymbol(node->callee()->AsIdentifier()->name()->ToSlice());
                if (!sym) {
                    Feedback()->Printf(node->callee()->source_position(), "Symbol `%s' not found",
                                       node->callee()->AsIdentifier()->name()->data());
                    return -1;
                }
                ast = sym;
                if (ProcessDependencySymbolIfNeeded(ast) < 0) {
                    return -1;
                }
            } break;
            case Node::kDot: {
                auto dot = node->callee()->AsDot();
                //printd("%s", dot->field()->data());
                if (auto id = dot->primary()->AsIdentifier()) {
                    auto [primary, _] = location_->FindSymbol(id->name()->ToSlice());
                    if (primary) {
                        is_others_expr = true;
                    } else {
                        ast = ResolveDotSymbol("Calling", node->callee()->AsDot(), Node::kMaxKinds);
                    }
                } else {
                    is_others_expr = true;
                }
            } break;
            case Node::kInstantiation:
                ast = Instantiate("Calling", node->callee()->AsInstantiation(), Node::kMaxKinds);
                break;
            case Node::kResolving:
                ast = ReduceResolving(node->callee()->AsResolving());
                break;
            default:
                is_others_expr = true;
                break;
        }
        
        Type *type = nullptr;
        if (!is_others_expr) {
            if (!ast) {
                return -1;
            }
            if (ast->IsClassDefinition() || ast->IsStructDefinition()) { // Is call constructor
                auto def = down_cast<IncompletableDefinition>(ast);
                std::vector<Type *> args;
                for (auto arg : node->args()) {
                    if (Reduce(arg, &args) < 0) {
                        return -1;
                    }
                }
                
                if (args.size() != def->parameters_size()) {
                    Feedback()->Printf(node->source_position(), "Unexpected function number of parameters, %zd vs %zd",
                                       def->parameters_size(), args.size());
                    return -1;
                }
                
                for (auto i = 0; i < def->parameters_size(); i++) {
                    Type *param = nullptr;
                    if (def->parameter(i).field_declaration) {
                        auto item = down_cast<VariableDeclaration::Item>(def->field(def->parameter(i).as_field).declaration->AtItem(0));
                        param = LinkType(item->Type());
                        item->set_type(param);
                    } else {
                        param = LinkType(def->parameter(i).as_parameter->Type());
                        def->parameter(i).as_parameter->set_type(param);
                    }
                    bool unlinked = false;
                    if (!param->Acceptable(args[i], &unlinked)) {
                        Feedback()->Printf(node->source_position(), "Unexpected constructor parameter[%d] type: `%s', "
                                           "actual is `%s'", i, param->ToString().c_str(), args[i]->ToString().c_str());
                        return -1;
                    }
                    DCHECK(!unlinked);
                }
                
                if (def->IsClassDefinition()) {
                    return Returning(new (arena_) ClassType(arena_, def->AsClassDefinition(), def->source_position()));
                } else {
                    return Returning(new (arena_) StructType(arena_, def->AsStructDefinition(), def->source_position()));
                }
            }
            
            if (auto val = ast->AsVariableDeclaration(); val && val->owns()->IsEnumDefinition()) {
                std::vector<Type *> args;
                for (auto arg : node->args()) {
                    if (Reduce(arg, &args) < 0) {
                        return -1;
                    }
                }
                
                if (args.size() != val->ItemSize()) {
                    Feedback()->Printf(node->source_position(), "Unexpected enum value number of parameters, %zd vs %zd",
                                       val->ItemSize(), args.size());
                    return -1;
                }
                
                for (auto i = 0; i < val->ItemSize(); i++) {
                    auto item = val->variable(i);
                    Type *param = LinkType(DCHECK_NOTNULL(item->type()));
                    if (!param) {
                        return -1;
                    }
                    item->set_type(param);

                    bool unlinked = false;
                    if (!param->Acceptable(args[i], &unlinked)) {
                        Feedback()->Printf(node->source_position(), "Unexpected enum value parameter[%d] type: `%s', "
                                           "actual is `%s'", i, param->ToString().c_str(), args[i]->ToString().c_str());
                        return -1;
                    }
                    DCHECK(!unlinked);
                }
                
                auto def = val->owns()->AsEnumDefinition();
                return Returning(new (arena_) EnumType(arena_, def, def->source_position()));
            }
            
            //DCHECK(ast->IsFunctionDeclaration());
            if (auto fun = ast->AsFunctionDeclaration()) {
                type = fun->prototype();
            }
        }
        
        if (!type) {
            if (ReduceReturningOnlyOne(node->callee(), &type, "Too many values for calling") < 0) {
                return -1;
            }
            if (!type->IsFunctionPrototype()) {
                Feedback()->Printf(node->source_position(), "Invalid type: `%s' for calling", type->ToString().c_str());
                return -1;
            }
        }
        
        std::vector<Type *> args;
        for (auto arg : node->args()) {
            if (Reduce(arg, &args) < 0) {
                return -1;
            }
        }
        auto prototype = type->AsFunctionPrototype();
        if (!prototype->vargs() && args.size() != prototype->params_size()) {
            Feedback()->Printf(node->source_position(), "Unexpected function number of parameters, %zd vs %zd",
                               prototype->params_size(), args.size());
            return -1;
        }
        
        if (!LinkType(prototype)) {
            return -1;
        }
        for (auto i = 0; i < prototype->params_size(); i++) {
            Type *param = nullptr;
            if (Type::Is(prototype->param(i))) {
                param = static_cast<Type *>(prototype->param(i));
            } else {
                param = static_cast<VariableDeclaration::Item *>(prototype->param(i))->type();
            }
            
            bool unlinked = false;
            if (!param->Acceptable(args[i], &unlinked)) {
                Feedback()->Printf(node->source_position(), "Unexpected function parameter[%d] type: `%s', actual is `%s'",
                                   i, param->ToString().c_str(), args[i]->ToString().c_str());
                return -1;
            }
            DCHECK(!unlinked);
        }
        
        std::vector<Type *> results;
        for (auto type : prototype->return_types()) {
            results.push_back(type);
        }
        return Returning(results);
    }
    

    int VisitAssignment(Assignment *node) override {
        std::vector<Type *> rvals;
        for (auto rval : node->rvals()) {
            if (Reduce(rval, &rvals) < 0) {
                return -1;
            }
        }
        
        std::vector<Type *> lvals;
        for (auto lval : node->lvals()) {
            Statement *symbol = nullptr;
            if (auto id = lval->AsIdentifier()) {
                auto [ast, ns] = location_->FindSymbol(id->name()->ToSlice());
                symbol = ast;
                if (!symbol) {
                    Feedback()->Printf(node->source_position(), "Symbol %s not found", id->name()->data());
                    return -1;
                }
                if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
                    return -1;
                }
            } else if (auto dot = lval->AsDot()) {
                if (auto id = dot->primary()->AsIdentifier(); id && MaybePackageName(id)) {
                    auto file_scope = location_->NearlyFileUnitScope();
                    symbol = file_scope->FindExportSymbol(id->name()->ToSlice(), dot->field()->ToSlice());
                    if (!symbol) {
                        Feedback()->Printf(node->source_position(), "Symbol %s.%s not found",
                                           id->name()->data(), dot->field()->data());
                        return -1;
                    }
                    if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
                        return -1;
                    }
                } else {
                    Type *type = nullptr;
                    if (ReduceReturningOnlyOne(dot->primary(), &type, "Too many values for field getter") < 0) {
                        return -1;
                    }

                    const IncompletableDefinition *level = nullptr;
                    auto owns = DCHECK_NOTNULL(GetTypeSpecifiedDefinition(type));
                    if (auto def = owns->AsClassDefinition()) {
                        auto [ast, ns] = def->FindSymbolWithOwns(dot->field()->ToSlice());
                        symbol = ast;
                        level = ns;
                    } else if (auto def = owns->AsStructDefinition()) {
                        auto [ast, ns] = def->FindSymbolWithOwns(dot->field()->ToSlice());
                        symbol = ast;
                        level = ns;
                    } else {
                        Feedback()->Printf(node->source_position(), "Invalid rval");
                        return -1;
                    }
                    if (!CheckAccessable(down_cast<IncompletableDefinition>(owns), symbol, level,
                                         node->source_position())) {
                        return -1;
                    }
                }
            } else if (auto indexed_get = lval->AsIndexedGet()) {
                //UNREACHABLE();
                auto lhs = indexed_get->primary();
                Type *type = nullptr;
                if (ReduceReturningOnlyOne(lhs, &type) < 0) {
                    return -1;
                }
                if (!type->IsArrayType()) {
                    Feedback()->Printf(node->source_position(), "Not array type with `[index]' indexed getting");
                    return -1;
                }

                std::vector<Type *> indices;
                for (auto index : indexed_get->indexs()) {
                    if (Reduce(index, &indices) < 0) {
                        return -1;
                    }
                }
                
                ArrayType *ar = DCHECK_NOTNULL(type->AsArrayType());
                if (indices.size() != ar->dimension_count()) {
                    Feedback()->Printf(node->source_position(), "Bad indices number, expected: %zd, need: %d",
                                       indices.size(), ar->dimension_count());
                    return -1;
                }
                
                for (auto index : indices) {
                    if (index->primary_type() != Type::kType_i32 && index->primary_type() != Type::kType_u32) {
                        Feedback()->Printf(node->source_position(), "Bad index type, expected: %s, need: i32/u32",
                                           index->ToString().c_str());
                        return -1;
                    }
                }
                
            
                lvals.push_back(ar->element_type());
            } else {
                Feedback()->Printf(node->source_position(), "Invalid rval");
                return -1;
            }

            if (symbol) {
                if (auto var = symbol->AsVariableDeclaration()) {
                    if (!node->initial() && var->constraint() == VariableDeclaration::kVal) {
                        Feedback()->Printf(node->source_position(), "Write `val' constraint value");
                        return -1;
                    }
                    lvals.push_back(var->Type());
                } else {
                    auto item = down_cast<VariableDeclaration::Item>(symbol);
                    if (!item->owns()) {
                        Feedback()->Printf(node->source_position(), "Write `val' constraint value");
                        return -1;
                    }
                    if (!node->initial() && item->owns()->constraint() == VariableDeclaration::kVal) {
                        Feedback()->Printf(node->source_position(), "Write `val' constraint value");
                        return -1;
                    }
                    lvals.push_back(item->Type());
                }
            }
        }
        
        if (lvals.size() != rvals.size()) {
            Feedback()->Printf(node->source_position(), "Unexpected number of lvals, %zd vs %zd", lvals.size(),
                               rvals.size());
            return -1;
        }
        for (auto i = 0; i < lvals.size(); i++) {
            bool unlinked = false;
            if (!DCHECK_NOTNULL(lvals[i])->Acceptable(rvals[i], &unlinked)) {
                Feedback()->Printf(node->source_position(), "Unexpected lvals[%d] type `%s', %s", i,
                                   lvals[i]->ToString().c_str(), rvals[i]->ToString().c_str());
                return -1;
            }
            DCHECK(!unlinked);
        }
        return Returning(Unit());
    }
    
    int VisitReturn(class Return *node) override {
        std::vector<Type *> rets;
        for (auto ret : node->returnning_vals()) {
            if (Reduce(ret, &rets) < 0) {
                return -1;
            }
        }
        
        auto fun_scope = DCHECK_NOTNULL(location_->NearlyFunctionScope());
        if (fun_scope->fun_is_reducing()) {
            Feedback()->Printf(node->source_position(), "Return vals for reducing(`->') function");
            return -1;
        }
        
        auto prototype = fun_scope->prototype();
        if (rets.size() != prototype->return_types_size()) {
            Feedback()->Printf(node->source_position(), "Unexpected number of returning types, %zd vs %zd",
                               prototype->return_types_size(), rets.size());
            return -1;
        }
        
        for (auto i = 0; i < prototype->return_types_size(); i++) {
            bool unlinked = false;
            if (!prototype->return_type(i)->Acceptable(rets[i], &unlinked)) {
                Feedback()->Printf(node->source_position(), "Unexpected returning[%d] type `%s', actual is `%s'", i,
                                   prototype->return_type(i)->ToString().c_str(), rets[i]->ToString().c_str());
                return -1;
            }
            DCHECK(!unlinked);
        }
        return Returning(Unit());
    }
    
    int VisitWhileLoop(WhileLoop *node) override {
        return VisitConditionLoop(node);
    }

    int VisitUnlessLoop(UnlessLoop *node) override {
        return VisitConditionLoop(node);
    }
    
    int VisitConditionLoop(ConditionLoop *node) {
        BlockScope scope(&location_, kLoopBlock, node);
        if (node->initializer()) {
            if (Reduce(node->initializer()) < 0) {
                return -1;
            }
        }
        
        Type *type = nullptr;
        if (node->execute_first()) {
            if (Reduce(node->body()) < 0) {
                return -1;
            }
            if (ReduceReturningAtLeastOne(node->condition(), &type) < 0) {
                return -1;
            }
        } else {
            if (ReduceReturningAtLeastOne(node->condition(), &type) < 0) {
                return -1;
            }
            if (Reduce(node->body()) < 0) {
                return -1;
            }
        }
        if (type->IsNotConditionVal()) {
            Feedback()->Printf(node->condition()->source_position(), "Condition must be bool expression or option");
            return -1;
        }
        return Returning(Unit());
    }

    int VisitForeachLoop(ForeachLoop *node) override {
        BlockScope scope(&location_, kLoopBlock, node);
        
        Type *iteration_type = nullptr;
        switch (node->iteration()) {
            case ForeachLoop::kIterator: {
                Type *type = nullptr;
                if (ReduceReturningAtLeastOne(node->iterable(), &type) < 0) {
                    return -1;
                }
                iteration_type = GetIterationType(type);
            } break;
                
            case ForeachLoop::kOpenBound:
            case ForeachLoop::kCloseBound: {
                std::vector<Type *> types;
                auto range = node->range();
                if (Reduce(range.lower, &types) < 0 || Reduce(range.upper, &types) < 0) {
                    return -1;
                }
                if (!types[0]->IsIntegral() || !types[1]->IsIntegral()) {
                    Feedback()->Printf(node->source_position(), "Invalid for-each bound type, need intergral type");
                    return -1;
                }
                iteration_type = ReduceType(types[0], types[1]);
            } break;
                
            default:
                UNREACHABLE();
                return -1;
        }
        
        auto var = new (arena_) VariableDeclaration(arena_, true, VariableDeclaration::kVal,
                                                    node->iterative_destination()->name(), iteration_type,
                                                    node->iterative_destination()->source_position());
        node->set_iterative_destination_var(var);
        scope.FindOrInsertSymbol(var->AtItem(0)->Identifier()->ToSlice(), var->AtItem(0));

        if (Reduce(node->body()) < 0) {
            return -1;
        }
        return Returning(Unit());
    }
    
    int VisitBreak(Break *node) override {
        auto scope = location_->NearlyBlockScope(kLoopBlock);
        if (!scope) {
            Feedback()->Printf(node->source_position(), "Break at loop outside");
            return -1;
        }
        return Returning(Unit());
    }

    int VisitContinue(Continue *node) override {
        auto scope = location_->NearlyBlockScope(kLoopBlock);
        if (!scope) {
            Feedback()->Printf(node->source_position(), "Continue at loop outside");
            return -1;
        }
        return Returning(Unit());
    }

    int VisitThrow(Throw *node) override {
        auto scope = location_->NearlyFunctionScope();
        if (!scope) {
            Feedback()->Printf(node->source_position(), "Throw at function outside");
            return -1;
        }
        Type *type = nullptr;
        if (ReduceReturningOnlyOne(node->throwing_val(), &type, "More than one throwing values") < 0) {
            return -1;
        }
        
        const auto throwable = FindSymbol(kLangPackageFullName, kThrowableClassName)->AsClassDefinition();
        if (!type->IsClassType() || type->AsClassType()->definition()->IsNotBaseOf(throwable)) {
            Feedback()->Printf(node->source_position(), "Throwing value must base of `Exception`");
            return -1;
        }
        
        return Returning(Unit());
    }
    
    int VisitCasting(Casting *node) override {
        auto dest = LinkType(node->destination());
        if (!dest) {
            return -1;
        }
        node->set_destination(dest);
        Type *src = nullptr;
        if (ReduceReturningOnlyOne(node->source(), &src, "More than one casting values") < 0) {
            return -1;
        }
        if (CastingFeasibilityTest(dest, src, node->source_position()) < 0) {
            return -1;
        }
        return Returning(node->destination());
    }
    
    int VisitTesting(Testing *node) override {
        auto dest = LinkType(node->destination());
        if (!dest) {
            return -1;
        }
        node->set_destination(dest);
        Type *src = nullptr;
        if (ReduceReturningOnlyOne(node->source(), &src, "More than one casting values") < 0) {
            return -1;
        }
        if (CastingFeasibilityTest(dest, src, node->source_position()) < 0) {
            return -1;
        }
        return Returning(Bool());
    }
    
    int VisitOptionLiteral(OptionLiteral *node) override {
        if (node->type()) {
            return Returning(node->type());
        }
        if (node->is_some()) {
            Type *type = nullptr;
            if (ReduceReturningAtLeastOne(node->value(), &type) < 0) {
                return -1;
            }
            type = new (arena_) OptionType(arena_, type, node->source_position());
            node->set_type(type);
            return Returning(type);
        }
        return Returning(node->type());
    }
    
    int VisitAssertedGet(AssertedGet *node) override {
        Type *type = nullptr;
        if (ReduceReturningOnlyOne(node->operand(), &type) < 0) {
            return -1;
        }
        if (!type->IsOptionType()) {
            Feedback()->Printf(node->source_position(), "Attempt DCHECKed-getting for non-option type: `%s'",
                               type->ToString().c_str());
            return -1;
        }
        return Returning(type->AsOptionType()->element_type());
    }
    
    int VisitOr(Or *node) override {
        Type *lhs = nullptr, *rhs = nullptr;
        if (ReduceReturningOnlyOne(node->lhs(), &lhs) < 0 || ReduceReturningOnlyOne(node->rhs(), &rhs) < 0) {
            return -1;
        }
        if (lhs->primary_type() == Type::kType_bool) {
            return Returning(lhs);
        }
        if (rhs->primary_type() == Type::kType_bool) {
            return Returning(rhs);
        }
        return Returning(Bool());
    }

    int VisitAnd(And *node) override {
        Type *lhs = nullptr, *rhs = nullptr;
        if (ReduceReturningOnlyOne(node->lhs(), &lhs) < 0 || ReduceReturningOnlyOne(node->rhs(), &rhs) < 0) {
            return -1;
        }
        if (lhs->primary_type() == Type::kType_bool) {
            return Returning(lhs);
        }
        if (rhs->primary_type() == Type::kType_bool) {
            return Returning(rhs);
        }
        return Returning(Bool());
    }
    
    int VisitNot(Not *node) override {
        Type *type = nullptr;
        if (Reduce(node->operand()) < 0) {
            return -1;
        }
        if (type->primary_type() == Type::kType_bool) {
            return Returning(type);
        }
        return Returning(Bool());
    }
    
    int VisitArrayInitializer(ArrayInitializer *node) override {
        //std::vector<int> dimensions_capacities;
        if (!node->type()) {
            int dimensions_count = kMaxArrayInitializerDims;
            auto type = ReduceArrayDimension(node->dimensions(), nullptr/*qualified*/, kMaxArrayInitializerDims,
                                             dimensions_count, node->source_position());
            if (!type) {
                return -1;
            }
            //printd("%s", type->ToString().c_str());
            if (auto ar = type->AsArrayType()) {
                dimensions_count = ar->GetActualDimensionCount() + 1;
            } else {
                dimensions_count = 1;
            }
            node->set_type(new (arena_) ArrayType(arena_, type, dimensions_count, node->source_position()));
            node->set_dimension_count(dimensions_count);
            InitArrayCapacitiesIfNeeded(node->dimensions(), 0, node->type()->AsArrayType()->dimension_count(),
                                        node->type()->AsArrayType());
            return Returning(node->type());
        }

        auto ar = DCHECK_NOTNULL(LinkType(node->type())->AsArrayType());
        node->set_type(ar);
        if (ar->HasCapacities()) {
            DCHECK(node->filling_value());
            Type *filling = nullptr;
            if (ReduceReturningOnlyOne(node->filling_value(), &filling) < 0) {
                return -1;
            }
            bool unlinked = false;
            auto elem_ty = ar->GetActualElementType();
            if (!elem_ty->Acceptable(filling, &unlinked)) {
                Feedback()->Printf(node->source_position(), "Unexpected array element type: `%s', filling value is `%s'",
                                   elem_ty->ToString().c_str(), filling->ToString().c_str());
                return -1;
            }
            DCHECK(!unlinked);
            return Returning(ar);
        }
        
        // int[,][][,,]
        // { ---+ array<array> 2
        //   { --/
        //     { ----+ array<array> 1
        //        {-------------------+ array<int> 3
        //           {----------------/
        //               {0}---------/
        //           }
        //        }
        //     }
        //   }
        // }
        //  array<array> 2
        //    array<array> 1
        //      array<int> 3
        auto rv = ReduceArrayDimension(node->dimensions(), ar->element_type(), ar->GetActualDimensionCount(),
                                       ar->dimension_count(), node->source_position());
        if (!rv) {
            return -1;
        }
        bool unlinked = false;
        if (!ar->element_type()->Acceptable(rv, &unlinked)) {
            Feedback()->Printf(node->source_position(), "Unexpected array element type: `%s', element is `%s'",
                               ar->element_type()->ToString().c_str(), rv->ToString().c_str());
            return -1;
        }
        DCHECK(!unlinked);
        InitArrayCapacitiesIfNeeded(node->dimensions(), 0, ar->dimension_count(), ar);
        return Returning(ar);
    }
    
    int InitArrayCapacitiesIfNeeded(const base::ArenaVector<AstNode *> &dimensions, int level,
                                    int dimension_count,
                                    ArrayType *input) {
        DCHECK(level >= 0);
        (*input->mutable_dimension_capacitys())[level] =
            new (arena_) cpl::IntLiteral(arena_, static_cast<int>(dimensions.size()), input->source_position());

        if (dimensions.empty()) {
            return 0;
        }
        
        if (dimension_count == 1) {
            if (auto init = dimensions.front()->AsArrayInitializer()) {
                DCHECK(input->element_type()->IsArrayType());
                auto ar = input->element_type()->AsArrayType();
                InitArrayCapacitiesIfNeeded(init->dimensions(), 0, ar->dimension_count(), ar);
            }
            return 0;
        }
        DCHECK(dimension_count > 1);
        auto init = DCHECK_NOTNULL(dimensions.front()->AsArrayInitializer());
        //auto ar = DCHECK_NOTNULL(input->element_type()->AsArrayType());
        return InitArrayCapacitiesIfNeeded(init->dimensions(), level + 1, dimension_count - 1, input);
    }
    
    int VisitIfExpression(IfExpression *node) override {
        BlockScope scope(&location_, kBranchBlock, node);
        const auto number_of_branchs = 1 + (node->else_clause() ? 1 : 0);
        std::unique_ptr<std::vector<Type *>[]> branchs_types(new std::vector<Type *>[number_of_branchs]);
        if (node->initializer()) {
            if (Reduce(node->initializer()) < 0) {
                return -1;
            }
        }
        
        Type *type = nullptr;
        if (ReduceReturningAtLeastOne(node->condition(), &type) < 0) {
            return -1;
        }
        if (type->IsNotConditionVal()) {
            Feedback()->Printf(node->condition()->source_position(), "Condition must be bool expression or option");
            return -1;
        }

        if (Reduce(node->then_clause(), &branchs_types[0]) < 0) {
            return -1;
        }

        if (node->else_clause()) {
            if (Reduce(node->else_clause(), &branchs_types[1]) < 0) {
                return -1;
            }
        }

        std::vector<Type *> results;
        if (ReduceBranchsTypes(branchs_types.get(), number_of_branchs, 1, &results, node->source_position()) < 0) {
            return -1;
        }
        for (auto type : results) { node->mutable_reduced_types()->push_back(type); }
        return Returning(results);
    }
    
    int VisitWhenExpression(WhenExpression *node) override {
        BlockScope scope(&location_, kBranchBlock, node);
        const auto number_of_branchs = node->case_clauses_size() + (node->else_clause() ? 1 : 0);
        std::unique_ptr<std::vector<Type *>[]> branchs_types(new std::vector<Type *>[number_of_branchs]);
        
        if (node->initializer()) {
            if (Reduce(node->initializer()) < 0) {
                return -1;
            }
        }
        Type *dest_type = nullptr;
        if (ReduceReturningAtLeastOne(node->destination(), &dest_type) < 0) {
            return -1;
        }

        EnumDefinition *enum_def = nullptr;
        if (dest_type->category() == Type::kEnum) {
            enum_def = dest_type->AsEnumType()->definition();
        }
        for (size_t i = 0; i < node->case_clauses_size(); i++) {
            if (auto clause = WhenExpression::ExpectValuesCase::Cast(node->case_clause(i))) {
                for (auto value : clause->match_values()) {
                    if (enum_def) {
                        if (ProcessEnumValueMatching(value, clause, enum_def) < 0) {
                            return -1;
                        }
                        continue;
                    }
                    
                    std::vector<Type *> types;
                    if (Reduce(value, &types) < 0) {
                        return -1;
                    }
                    
                    for (auto value_type : types) {
                        bool unlinked = false;
                        if (!dest_type->Acceptable(value_type, &unlinked)) {
                            Feedback()->Printf(value->source_position(), "Unacceptable matching value type: `%s'"
                                               "destination is `%s'", value_type->ToString().c_str(),
                                               dest_type->ToString().c_str());
                            return -1;
                        }
                        DCHECK(!unlinked);
                    }
                }
                
                if (Reduce(clause->then_clause(), &branchs_types[i]) < 0) {
                    return -1;
                }
            } else if (auto clause = WhenExpression::TypeTestingCase::Cast(node->case_clause(i))) {
                auto match_type = LinkType(clause->match_type());
                if (!match_type) {
                    return -1;
                }
                clause->set_match_type(match_type);
                if (CastingFeasibilityTest(match_type, dest_type, clause->source_position()) < 0) {
                    return -1;
                }
                
                BlockScope inner_scope(&location_, kPlainBlock, clause->then_clause());
                auto dummy = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal,
                                                              clause->name()->name(), match_type,
                                                              clause->name()->source_position());
                inner_scope.FindOrInsertSymbol(clause->name()->name()->ToSlice(), dummy->AtItem(0));
                
                if (Reduce(clause->then_clause(), &branchs_types[i]) < 0) {
                    return -1;
                }
                continue;
            } else if (auto clause = WhenExpression::BetweenToCase::Cast(node->case_clause(i))) {
                Type *lower_type = nullptr, *upper_type = nullptr;
                if (ReduceReturningOnlyOne(clause->lower(), &lower_type) < 0 ||
                    ReduceReturningOnlyOne(clause->upper(), &upper_type) < 0) {
                    return -1;
                }
                if (!lower_type->IsNumber() && !upper_type->IsNumber()) {
                    Feedback()->Printf(clause->source_position(), "Between-and case needs number type, actual is `%s',"
                                       " `%s'", lower_type->ToString().c_str(), upper_type->ToString().c_str());
                    return -1;
                }
                if (lower_type->primary_type() != upper_type->primary_type()) {
                    Feedback()->Printf(clause->source_position(), "Between-and case bound has different type: `%s',"
                                       " `%s'", lower_type->ToString().c_str(), upper_type->ToString().c_str());
                    return -1;
                }
                
                bool unlinked = false;
                if (!dest_type->Acceptable(lower_type, &unlinked)) {
                    Feedback()->Printf(clause->lower()->source_position(), "Unacceptable bundle type: `%s'"
                                       "destination is `%s'", lower_type->ToString().c_str(),
                                       dest_type->ToString().c_str());
                    return -1;
                }
                DCHECK(!unlinked);
                
                if (Reduce(clause->then_clause(), &branchs_types[i]) < 0) {
                    return -1;
                }
            } else if (auto clause = WhenExpression::StructMatchingCase::Cast(node->case_clause(i))) {
                auto type = LinkType(clause->match_type());
                clause->set_match_type(type);
                
                IncompletableDefinition *def = nullptr;
                switch (type->kind()) {
                    case Node::kStructType:
                        def = type->AsStructType()->definition();
                        break;
                    case Node::kClassType:
                        def = type->AsClassType()->definition();
                        break;
                    default:
                        Feedback()->Printf(clause->source_position(), "Struct/Class: %s not found",
                                           type->ToString().c_str());
                        return -1;
                }
                

                for (auto id : clause->expecteds()) {
                    auto field = def->FindLocalSymbolOrNull(id->name()->ToSlice());
                    if (!field || field->IsFunctionDeclaration()) {
                        Feedback()->Printf(clause->source_position(), "Matching field: %s not found in %s",
                                           id->name()->data(), def->FullName().c_str());
                        return -1;
                    }

                    BlockScope inner_scope(&location_, kPlainBlock, clause->then_clause());
                    auto dummy = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal,
                                                                  id->name(), down_cast<Declaration>(field)->Type(),
                                                                  id->source_position());
                    inner_scope.FindOrInsertSymbol(id->name()->ToSlice(), dummy->AtItem(0));
                    
                    if (Reduce(clause->then_clause(), &branchs_types[i]) < 0) {
                        return -1;
                    }
                    continue;
                }
            } else {
                UNREACHABLE();
            }
        }
        if (node->else_clause()) {
            if (Reduce(node->else_clause(), &branchs_types[number_of_branchs - 1]) < 0) {
                return -1;
            }
        }
        
        std::vector<Type *> resutls;
        if (ReduceBranchsTypes(branchs_types.get(), number_of_branchs, node->case_clauses_size(),
                               &resutls, node->source_position()) < 0) {
            return -1;
        }
        for (auto type : resutls) { node->mutable_reduced_types()->push_back(type); }
        return Returning(resutls);
    }
    
    int VisitChannelInitializer(ChannelInitializer *node) override {
        auto type = LinkType(node->type());
        node->set_type(type);
        
        if (ReduceReturningOnlyOne(node->capacity(), &type) < 0) {
            return -1;
        }
        if (!type->IsIntegral()) {
            Feedback()->Printf(node->capacity()->source_position(), "Incorrect type of channel capacity, need integral,"
                               "actual is `%s'", type->ToString().c_str());
            return -1;
        }
        
        return Returning(node->type());
    }
    
    int VisitStringTemplate(StringTemplate *node) override {
        Type *type = nullptr;
        for (auto part : node->parts()) {
            if (ReduceReturningOnlyOne(part, &type) < 0) {
                return -1;
            }
        }
        return Returning(StringTy());
    }
    
    int VisitInstantiation(Instantiation *node) override {
        Statement *ast = nullptr;
        if (ast = Instantiate("Instantiation", node, Node::kMaxKinds); !ast) {
            return -1;
        }
        return ast->Accept(this);
    }
    
    int VisitRunCoroutine(RunCoroutine *node) override {
        if (Reduce(node->entry()) < 0) {
            return -1;
        }
        return Returning(Unit());
    }
    
    int VisitTryCatchExpression(TryCatchExpression *node) override {
        const auto number_of_branchs = node->catch_clauses_size() + 1/*try block*/;
        std::unique_ptr<std::vector<Type *>[]> branchs_types(new std::vector<Type *>[number_of_branchs]);
        
        if (Reduce(node->try_block(), &branchs_types[0]) < 0) {
            return -1;
        }
        
        auto sym = FindGlobal(kLangPackageFullName, kThrowableClassName);
        DCHECK(sym.IsFound());
        auto throwable = DCHECK_NOTNULL(sym.ast->AsClassDefinition());
        
        std::map<ClassDefinition *, TryCatchExpression::CatchClause *> unique_types;
        for (auto i = 0; i < node->catch_clauses_size(); i++) {
            auto clause = node->catch_clause(i);
            auto matching_type = LinkType(clause->match_type());
            if (ClassType::DoNotClassBaseOf(matching_type, throwable)) {
                Feedback()->Printf(clause->source_position(), "Catch type: `%s' is not base of `Throwable' class",
                                   matching_type->ToString().c_str());
                return -1;
            }
            if (auto iter = unique_types.find(matching_type->AsClassType()->definition()); iter != unique_types.end()) {
                Feedback()->Printf(clause->source_position(), "Catch type: `%s' is duplicated, prev one: %s",
                                   matching_type->ToString().c_str(),
                                   iter->second->source_position().ToString().c_str());
                return -1;
            }
            unique_types[matching_type->AsClassType()->definition()] = clause;
            clause->set_match_type(matching_type);
            
            BlockScope scope(&location_, kPlainBlock, clause->then_clause());
            auto stub = new (arena_) VariableDeclaration(arena_, false, VariableDeclaration::kVal,
                                                         clause->name()->name(), matching_type,
                                                         clause->name()->source_position());
            scope.FindOrInsertSymbol(stub->Identifier()->ToSlice(), stub->AtItem(0));
            
            if (Reduce(clause->then_clause(), &branchs_types[i + 1]) < 0) {
                return -1;
            }
        }
        
        std::vector<Type *> results;
        if (ReduceBranchsTypes(branchs_types.get(), number_of_branchs, number_of_branchs - 1, &results,
                               node->source_position()) < 0) {
            return -1;
        }
        
        if (node->finally_block() && Reduce(node->finally_block()) < 0) {
            return -1;
        }
        for (auto type : results) { node->mutable_reduced_types()->push_back(type); }
        return Returning(results);
    }
    
    enum OperatingKind {
        kNumberTy,
        kIntegralTyOnly,
        kComparableTy,
    };
    
    int VisitIndexedGet(IndexedGet *node) override {
        Type *type = nullptr;
        if (ReduceReturningOnlyOne(node->primary(), &type) < 0) {
            return -1;
        }
        if (!type->IsArrayType()) {
            Feedback()->Printf(node->source_position(), "Attempt `[]' operator for none-array type: `%s'",
                               type->ToString().c_str());
            return -1;
        }
        auto ar = type->AsArrayType();
        
        std::vector<Type *> indices;
        for (auto index : node->indexs()) {
            if (Reduce(index, &indices) < 0) {
                return -1;
            }
        }
        if (indices.size() != ar->dimension_count()) {
            Feedback()->Printf(node->source_position(), "Bad indices number, expected: %zd, needed: %d",
                               indices.size(), ar->dimension_count());
            return -1;
        }
        for (auto index : indices) {
            if (index->primary_type() != Type::kType_u32 && index->primary_type() != Type::kType_i32) {
                Feedback()->Printf(node->source_position(), "Bad index type, expected: %s, needed: i32/u32",
                                   index->ToString().c_str());
                return -1;
            }
        }
        
        return Returning(ar->element_type());
    }

    int VisitAdd(Add *node) override { return ReduceBinaryExpression("+", node, kNumberTy); }
    int VisitDiv(Div *node) override { return ReduceBinaryExpression("/", node, kNumberTy); }
    int VisitMod(Mod *node) override { return ReduceBinaryExpression("%", node, kIntegralTyOnly); }
    int VisitMul(Mul *node) override { return ReduceBinaryExpression("*", node, kNumberTy); }
    int VisitSub(Sub *node) override { return ReduceBinaryExpression("-", node, kNumberTy); }
    
    int VisitNegative(Negative *node) override {
        Type *type = nullptr;
        if (ReduceReturningOnlyOne(node->operand(), &type) < 0) {
            return -1;
        }
        if (!type->IsFloating() && !type->IsSignedIntegral()) {
            Feedback()->Printf(node->source_position(), "Incorrect type of operand, -`%s'", type->ToString().c_str());
            return -1;
        }
        return Returning(type);
    }
    
    int VisitLess(Less *node) override { return ReduceBinaryExpression("<", node, kComparableTy); }
    int VisitLessEqual(LessEqual *node) override { return ReduceBinaryExpression("<=", node, kComparableTy); }
    int VisitGreater(Greater *node) override { return ReduceBinaryExpression(">", node, kComparableTy); }
    int VisitGreaterEqual(GreaterEqual *node) override { return ReduceBinaryExpression(">=", node, kComparableTy); }
    int VisitEqual(Equal *node) override { return ReduceBinaryExpression("==", node, kComparableTy); }
    int VisitNotEqual(NotEqual *node) override { return ReduceBinaryExpression("!=", node, kComparableTy); }
    
    int VisitRecv(Recv *node) override {
        Type *type = nullptr;
        if (ReduceReturningOnlyOne(node->operand(), &type) < 0) {
            return -1;
        }
        if (!type->IsChannelType()) {
            Feedback()->Printf(node->source_position(), "Incorrect type of operand, <-`%s'", type->ToString().c_str());
            return -1;
        }
        auto chan = type->AsChannelType();
        if (!chan->CanRead()) {
            Feedback()->Printf(node->source_position(), "Attempt read unreadable channel");
            return -1;
        }
        return Returning({chan->element_type(), Bool()});
    }
    
    int VisitSend(Send *node) override {
        Type *receiver = nullptr, *sendee = nullptr;
        if (ReduceReturningOnlyOne(node->lhs(), &receiver) < 0 || ReduceReturningOnlyOne(node->rhs(), &sendee) < 0) {
            return -1;
        }
        if (!receiver->IsChannelType()) {
            Feedback()->Printf(node->source_position(), "Incorrect type of operand, `%s'<-`%s'",
                               receiver->ToString().c_str(), sendee->ToString().c_str());
            return -1;
        }
        auto chan = receiver->AsChannelType();
        if (!chan->CanWrite()) {
            Feedback()->Printf(node->source_position(), "Attempt write unwritable channel");
            return -1;
        }
        bool unlinked = false;
        if (!chan->element_type()->Acceptable(sendee, &unlinked)) {
            Feedback()->Printf(node->source_position(), "Channel does not accept: `%s' <= `%s'",
                               chan->element_type()->ToString().c_str(),
                               sendee->ToString().c_str());
            return -1;
        }
        return Returning(Bool());
    }
    
    
    int VisitBitwiseOr(BitwiseOr *node) override { return ReduceBinaryExpression("|", node, kIntegralTyOnly); }
    int VisitBitwiseAnd(BitwiseAnd *node) override { return ReduceBinaryExpression("&", node, kIntegralTyOnly); }
    int VisitBitwiseShl(BitwiseShl *node) override { return ReduceBinaryExpression("<<", node, kIntegralTyOnly); }
    int VisitBitwiseShr(BitwiseShr *node) override { return ReduceBinaryExpression(">>", node, kIntegralTyOnly); }
    int VisitBitwiseXor(BitwiseXor *node) override { return ReduceBinaryExpression("^", node, kIntegralTyOnly); }
    
    int VisitBitwiseNegative(BitwiseNegative *node) override {
        Type *type = nullptr;
        if (ReduceReturningOnlyOne(node->operand(), &type) < 0) {
            return -1;
        }
        if (!type->IsIntegral()) {
            Feedback()->Printf(node->source_position(), "Incorrect type of operand, ~`%s'", type->ToString().c_str());
            return -1;
        }
        return Returning(type);
    }
    
    int VisitF32Literal(F32Literal *node) override { return Returning(node->type()); }
    int VisitF64Literal(F64Literal *node) override { return Returning(node->type()); }
    int VisitI8Literal(I8Literal *node) override { return Returning(node->type()); }
    int VisitU8Literal(U8Literal *node) override { return Returning(node->type()); }
    int VisitI16Literal(I16Literal *node) override { return Returning(node->type()); }
    int VisitU16Literal(U16Literal *node) override { return Returning(node->type()); }
    int VisitI32Literal(I32Literal *node) override { return Returning(node->type()); }
    int VisitU32Literal(U32Literal *node) override { return Returning(node->type()); }
    int VisitIntLiteral(IntLiteral *node) override { return Returning(node->type()); }
    int VisitUIntLiteral(UIntLiteral *node) override { return Returning(node->type()); }
    int VisitI64Literal(I64Literal *node) override { return Returning(node->type()); }
    int VisitU64Literal(U64Literal *node) override { return Returning(node->type()); }
    int VisitBoolLiteral(BoolLiteral *node) override { return Returning(node->type()); }
    int VisitUnitLiteral(UnitLiteral *node) override { return Returning(Unit()); }
    int VisitEmptyLiteral(EmptyLiteral *node) override { UNREACHABLE(); }
    int VisitStringLiteral(StringLiteral *node) override { return Returning(node->type()); }
    int VisitCharLiteral(CharLiteral *node) override { return Returning(node->type()); }
    
    int VisitAnnotationDefinition(AnnotationDefinition *node) override { UNREACHABLE(); }
    int VisitAnnotationDeclaration(AnnotationDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotation(Annotation *node) override { UNREACHABLE(); }
    
private:
    int ProcessEnumValueMatching(Expression *value, WhenExpression::ExpectValuesCase *clause,
                                 EnumDefinition *enum_def) {
        if (auto id = value->AsIdentifier()) {
            auto ast = enum_def->FindSymbolOrNull(id->name()->ToSlice());
            if (!ast) {
                Feedback()->Printf(value->source_position(), "Enum value: `%s' not found in `%s'",
                                   id->name()->data(), enum_def->FullName().c_str());
                return -1;
            }
            if (!ast->IsVariableDeclaration()) {
                Feedback()->Printf(value->source_position(), "`%s' is not enum value in `%s'",
                                   id->name()->data(), enum_def->FullName().c_str());
                return -1;
            }
            return 0;
        }
        
        if (auto calling = value->AsCalling()) {
            auto id = calling->callee()->AsIdentifier();
            if (!id) {
                Feedback()->Printf(value->source_position(), "Invalid enum value matching in `%s'",
                                   enum_def->FullName().c_str());
                return -1;
            }
            
            auto ast = enum_def->FindSymbolOrNull(id->name()->ToSlice());
            if (!ast) {
                Feedback()->Printf(value->source_position(), "Enum value: `%s' not found in `%s'",
                                   id->name()->data(), enum_def->FullName().c_str());
                return -1;
            }
            auto value = ast->AsVariableDeclaration();
            if (!value) {
                Feedback()->Printf(value->source_position(), "`%s' is not enum value in `%s'",
                                   id->name()->data(), enum_def->FullName().c_str());
                return -1;
            }

            if (calling->args_size() != value->ItemSize()) {
                Feedback()->Printf(value->source_position(), "Invalid enum value: `%s' matching in `%s'",
                                   id->name()->data(), enum_def->FullName().c_str());
                return -1;
            }
            
            for (int i = 0; i < calling->args_size(); i++) {
                auto name = calling->arg(i)->AsIdentifier();
                if (!name) {
                    Feedback()->Printf(value->source_position(), "Invalid enum value: `%s' matching in `%s'",
                                       id->name()->data(), enum_def->FullName().c_str());
                }
                
                auto item = value->AtItem(i);
                location_->FindOrInsertSymbol(name->name()->ToSlice(), item);
            }
            
            return 0;
        }
        
        Feedback()->Printf(value->source_position(), "Invalid enum value matching in `%s'",
                           enum_def->FullName().c_str());
        return -1;
    }
    
    int ReduceBinaryExpression(const char *op, BinaryExpression *expr, OperatingKind kind) {
        Type *lhs = nullptr, *rhs = nullptr;
        if (ReduceReturningOnlyOne(expr->lhs(), &lhs) < 0 || ReduceReturningOnlyOne(expr->rhs(), &rhs) < 0) {
            return -1;
        }
        
        bool ok = true;
        switch (kind) {
            case kNumberTy:
                ok = lhs->IsNumber() && rhs->IsNumber();
                break;
            case kIntegralTyOnly:
                ok = lhs->IsIntegral() && rhs->IsIntegral();
                break;
            case kComparableTy:
                ok = lhs->IsComparable() && rhs->IsComparable();
                break;
            default:
                UNREACHABLE();
                break;
        }
        if (!ok) {
            Feedback()->Printf(expr->source_position(), "Incorrect type of operand, `%s' %s `%s'",
                               lhs->ToString().c_str(), op, rhs->ToString().c_str());
            return -1;
        }
        
        bool unlinked = false;
        if (!lhs->Acceptable(rhs, &unlinked) || !rhs->Acceptable(lhs, &unlinked)) {
            Feedback()->Printf(expr->source_position(), "Type of operand is not acceptable each other, `%s' <=> `%s'",
                               lhs->ToString().c_str(), rhs->ToString().c_str());
            return -1;
        }
        DCHECK(!unlinked);
        return Returning(kind == kComparableTy ? Bool() : lhs);
    }
    
    int ReduceBranchsTypes(const std::vector<Type *> branchs_rows[],
                           const size_t number_of_branchs,
                           const size_t number_of_branchs_without_else,
                           std::vector<Type *> *resutls,
                           const SourcePosition &source_position) {
        size_t max_cols = 0;
        bool all_unit = true;
        for (size_t i = 0; i < number_of_branchs; i++) {
            for (auto type : branchs_rows[i]) {
                if (type->primary_type() != Type::kType_unit) {
                    all_unit = false;
                }
            }
            max_cols = std::max(max_cols, branchs_rows[i].size());
        }
        if (all_unit) {
            return Returning(Unit());
        }
        
        std::vector<std::vector<Type *>> branchs_cols(max_cols);
        for (size_t i = 0; i < max_cols; i++) {
            auto col = &branchs_cols[i];
            for (size_t j = 0; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                if (j < number_of_branchs && i < branchs_rows[j].size()) {
                    col->push_back(branchs_rows[j][i]);
                } else {
                    col->push_back(Unit());
                }
            }
        }
        
        for (size_t i = 0; i < max_cols; i++) {
            bool all_unit = true;
            bool at_least_one_unit = false;
            
            for (size_t j = 0; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                if (branchs_cols[i][j]->primary_type() == Type::kType_unit) {
                    at_least_one_unit = true;
                } else {
                    all_unit = false;
                }
            }
            
            if (all_unit) {
                resutls->push_back(Unit());
                continue;
            }
            
            Type *reduced = nullptr;
            if (at_least_one_unit) {
                DCHECK(!all_unit);
                for (size_t j = 0; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                    if (branchs_cols[i][j]->primary_type() != Type::kType_unit) {
                        if (!reduced) {
                            reduced = branchs_cols[i][j];
                            continue;
                        }
                        auto ty = ReduceType(reduced, branchs_cols[i][j]);
                        if (!ty) {
                            Feedback()->Printf(source_position, "Unacceptable branches types `%s' <=> `%s'",
                                               reduced->ToString().c_str(), branchs_cols[i][j]->ToString().c_str());
                            return -1;
                        }
                        reduced = ty;
                    }
                }
                reduced = new (arena_) OptionType(arena_, reduced, reduced->source_position());
                resutls->push_back(reduced);
                continue;
            }
            
            reduced = branchs_cols[i][0];
            if (branchs_cols[i].size() == 1) {
                resutls->push_back(reduced);
                continue;
            }
            
            for (size_t j = 1; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                auto ty = ReduceType(reduced, branchs_cols[i][j]);
                if (!ty) {
                    Feedback()->Printf(source_position, "Unacceptable branches types `%s' <=> `%s'",
                                       reduced->ToString().c_str(), branchs_cols[i][j]->ToString().c_str());
                    return -1;
                }
                reduced = ty;
            }
            resutls->push_back(reduced);
        }
        return 0;
    }
    
    Type *ReduceArrayDimension(const base::ArenaVector<AstNode *> &dimension,
                               Type *qualified,
                               int dimensions_limit,
                               int dimensions_count,
                               const SourcePosition &source_position) {
        //if (dimensions_limit < *dimensions_count) { *dimensions_count = dimensions_limit; }
        
        if (dimensions_limit == 0) {
            Feedback()->Printf(source_position, "Max array initializer dimensons: %d", dimensions_count);
            return nullptr;
        }
        
        // {
        //     {1, 2},
        //     {1, 2}
        // }
        //
        // [0] {1, 2}, {1, 2} d=2
        // [0,1] {1,2}        d=1
        if (qualified) {
            Type *type = nullptr;
            if (auto ar = qualified->AsArrayType()) {
                for (auto elem : dimension) {
                    auto init = elem->AsArrayInitializer();
                    if (!init) {
                        Feedback()->Printf(source_position, "Incorrect array initializer dimensons: %d", dimensions_count);
                        return nullptr;
                    }
                    if (dimensions_count == 1) {
                        dimensions_count = qualified->IsArrayType() ? qualified->AsArrayType()->dimension_count() : 1;
                        qualified = ar->element_type();
                    } else {
                        dimensions_count--;
                        qualified = ar;
                    }
                    type = ReduceArrayDimension(init->dimensions(), qualified, dimensions_limit - 1, dimensions_count,
                                                elem->source_position());
                    init->set_type(new (arena_) ArrayType(arena_, type, dimensions_limit - dimensions_count,
                                                          init->source_position()));
                    init->set_dimension_count(ar->dimension_count());
                }
                return ar;
            } else {
                for (auto elem : dimension) {
                    if (ReduceReturningOnlyOne(elem, &type) < 0) {
                        return nullptr;
                    }
                    if (!ReduceType(qualified, type)) {
                        Feedback()->Printf(elem->source_position(), "Unexpected array type: `%s', element is `%s'",
                                           qualified->ToString().c_str(), type->ToString().c_str());
                        return nullptr;
                    }
                }
                return qualified;
            }
        } else {
            std::vector<Type *> types;
            for (auto elem : dimension) {
                size_t dim_size = 0;
                if (auto ar = elem->AsArrayInitializer()) {
                    if (dim_size != 0 && dim_size != ar->dimensions_size()) {
                        Feedback()->Printf(elem->source_position(), "Diffecent dimension size: %zd vs %zd", dim_size,
                                           ar->dimensions_size());
                        return nullptr;
                    }
                    auto type = ReduceArrayDimension(ar->dimensions(), qualified, dimensions_limit - 1, dimensions_count,
                                                     elem->source_position());
                    dim_size = ar->dimensions_size();
                    ar->set_type(new (arena_) ArrayType(arena_, type, 1, ar->source_position()));
                    ar->set_dimension_count(1);
                    types.push_back(ar->type());
                } else {
                    Type *type = nullptr;
                    if (ReduceReturningOnlyOne(elem, &type) < 0) {
                        return nullptr;
                    }
                    types.push_back(type);
                }
            }
            
            if (types.empty()) {
                Feedback()->Printf(source_position, "No element in array initializer");
                return nullptr;
            }
            if (types.size() == 1) {
                return types[0];
            }
            auto reduced = types[0];
            for (auto i = 1; i < types.size(); i++) {
                reduced = ReduceType(reduced, types[i]);
            }
            return reduced;
        }
    }
    
    int CastingFeasibilityTest(Type *dest, Type *src, const SourcePosition &source_position) {
        switch (Constants::HowToCasting(dest->primary_type(), src->primary_type())) {
            case CastingRule::DENY:
                goto not_allow;
                
            case CastingRule::ALLOW:
            case CastingRule::ALLOW_UNBOX:
            case CastingRule::ALLOW_TYPING:
            case CastingRule::ALLOW_TYPING_CONCEPT:
                break;
                
            case CastingRule::PROTOTYPE: {
                auto dest_fun = DCHECK_NOTNULL(dest->AsFunctionPrototype()),
                src_fun = DCHECK_NOTNULL(src->AsFunctionPrototype());
                if (dest_fun->signature() != src_fun->signature()) {
                    goto not_allow;
                }
            } break;
                
            case CastingRule::CONCEPT: {
                auto clazz = DCHECK_NOTNULL(src->AsClassType())->definition();
                auto interface = DCHECK_NOTNULL(dest->AsInterfaceType())->definition();
                if (clazz->IsNotConceptOf(interface)) {
                    goto not_allow;
                }
            } break;
                
            case CastingRule::OPTION_VALUE:
            case CastingRule::ELEMENT:
            case CastingRule::ELEMENT_IN_OUT: {
                bool unlinked = false;
                if (!dest->Acceptable(src, &unlinked)) {
                    goto not_allow;
                }
                DCHECK(!unlinked);
            } break;
                
            case CastingRule::I8_U8_CHAR_ARRAY_ONLY: {
                if (dest->primary_type() == Type::kType_string) {
                    if (auto src_ar = src->AsArrayType()) {
                        if (src_ar->dimension_count() != 1 ||
                            src_ar->element_type()->IsNotCharAndByte()) {
                            goto not_allow;
                        }
                    }
                } else if (auto dest_ar = dest->AsArrayType()) {
                    if (src->primary_type() == Type::kType_string) {
                        if (dest_ar->dimension_count() != 1 ||
                            dest_ar->element_type()->IsNotCharAndByte()) {
                            goto not_allow;
                        }
                    }
                }
            } break;
                
            case CastingRule::SELF_ONLY: {
                auto dest_if = DCHECK_NOTNULL(dest->AsInterfaceType())->definition();
                auto src_if = DCHECK_NOTNULL(src->AsInterfaceType())->definition();
                if (dest_if != src_if) {
                    goto not_allow;
                }
            } break;
                
            case CastingRule::CHILD_CLASS_ONLY: {
                auto dest_struct_ty = DCHECK_NOTNULL(dest->AsStructType());
                if (auto src_struct_ty = src->AsStructType()) {
                    if (src_struct_ty->definition()->IsNotBaseOf(dest_struct_ty->definition())) {
                        goto not_allow;
                    }
                } else {
                    goto not_allow;
                }
            } break;
                
            case CastingRule::CLASS_BASE_OF: {
                auto dest_class_ty = DCHECK_NOTNULL(dest->AsClassType());
                if (auto src_class_ty = src->AsClassType()) {
                    if (src_class_ty->definition()->IsNotBaseOf(dest_class_ty->definition()) &&
                        dest_class_ty->definition()->IsNotBaseOf(src_class_ty->definition())) {
                        goto not_allow;
                    }
                } else {
                    goto not_allow;
                }
            } break;
                
            default:
                UNREACHABLE();
                break;
        }
        
        return 0;
    not_allow:
        Feedback()->Printf(source_position, "Type casting is not allow, `%s' <= `%s'", dest->ToString().c_str(),
                           src->ToString().c_str());
        return -1;
    }
    
    bool CheckAccessable(IncompletableDefinition *owns, Statement *symbol, const IncompletableDefinition *level,
                         const SourcePosition &source_position) {
        Access access = kDefault;
        if (auto var = symbol->AsVariableDeclaration()) {
            access = var->access();
        } else if (auto fun = symbol->AsFunctionDeclaration()) {
            access = fun->access();
        } else if (auto item = down_cast<VariableDeclaration::Item>(symbol)) {
            access = DCHECK_NOTNULL(item->owns())->access();
        } else {
            UNREACHABLE();
        }
        if (access == kDefault) {
            access = kPublic;
        }
        
        auto def_scope = location_->NearlyDataDefinitionScope();
        auto in_def_scope = def_scope != nullptr && def_scope->definition() == owns;
        if (!in_def_scope) {
            if (access != kPublic) {
                Feedback()->Printf(source_position, "Access non-public field or method");
                return false;
            }
            return true;
        }
        
        if (level != def_scope->definition() && access != kProtected) {
            Feedback()->Printf(source_position, "Access private field or method");
            return false;
        }
        return true;
    }
    
    Type *ReduceType(Type *lhs, Type *rhs) {
        if (lhs->IsNumber() && rhs->IsNumber()) {
            auto ty = static_cast<Type::Primary>(Constants::ReduceNumberType(lhs->primary_type(), rhs->primary_type()));
            if (ty == lhs->primary_type()) {
                return lhs;
            }
            if (ty == rhs->primary_type()) {
                return rhs;
            }
            return new (arena_) Type(arena_, ty, lhs->source_position());
        }
        if (lhs->ToString() == rhs->ToString()) {
            return lhs;
        }

        switch (Constants::HowToCasting(lhs->primary_type(), rhs->primary_type())) {
            case CastingRule::DENY:
            case CastingRule::I8_U8_CHAR_ARRAY_ONLY:
                break;
                
            case CastingRule::ALLOW:
            case CastingRule::ALLOW_UNBOX:
            case CastingRule::ALLOW_TYPING:
            case CastingRule::ALLOW_TYPING_CONCEPT: {
                if (lhs->primary_type() == Type::kType_any) {
                    return lhs;
                }
                if (rhs->primary_type() == Type::kType_any) {
                    return rhs;
                }
            } break;
                
            case CastingRule::PROTOTYPE: {
                auto lhs_fun = DCHECK_NOTNULL(lhs->AsFunctionPrototype());
                auto rhs_fun = DCHECK_NOTNULL(rhs->AsFunctionPrototype());
                if (lhs_fun->signature() == rhs_fun->signature()) {
                    return lhs;
                }
                
            } break;
                
            case CastingRule::CONCEPT: {
                auto clazz = DCHECK_NOTNULL(rhs->AsClassType())->definition();
                auto interface = DCHECK_NOTNULL(lhs->AsInterfaceType())->definition();
                if (clazz->IsConceptOf(interface)) {
                    return lhs;
                }
            } break;
                
            case CastingRule::OPTION_VALUE:
            case CastingRule::ELEMENT:
            case CastingRule::ELEMENT_IN_OUT: {
                bool unlinked = false;
                if (lhs->Acceptable(rhs, &unlinked)) {
                    return lhs;
                }
                DCHECK(!unlinked);
            } break;
                
            case CastingRule::SELF_ONLY: {
                auto lhs_if = DCHECK_NOTNULL(lhs->AsInterfaceType())->definition();
                auto rhs_if = DCHECK_NOTNULL(rhs->AsInterfaceType())->definition();
                if (lhs_if == rhs_if) {
                    return lhs;
                }
            } break;
                
            case CastingRule::CHILD_CLASS_ONLY: {
                if (auto lhs_st = lhs->AsStructType()) {
                    if (auto rhs_st = rhs->AsStructType()) {
                        if (lhs_st->definition()->IsBaseOf(rhs_st->definition())) {
                            return rhs;
                        }
                        if (rhs_st->definition()->IsBaseOf(lhs_st->definition())) {
                            return lhs;
                        }
                    }
                }
            } break;
                
            case CastingRule::CLASS_BASE_OF: {
                if (auto lhs_ty = lhs->AsClassType()) {
                    if (auto rhs_ty = rhs->AsClassType()) {
                        if (lhs_ty->definition()->IsBaseOf(rhs_ty->definition())) {
                            return rhs;
                        }
                        if (rhs_ty->definition()->IsBaseOf(lhs_ty->definition())) {
                            return lhs;
                        }
                    }
                }
            } break;
                
            default:
                UNREACHABLE();
                break;
        }
        
        return Any();
    }
    
    Type *GetIterationType(Type *iterable) {
        switch (iterable->primary_type()) {
            case Type::kType_array: {
                auto at = iterable->AsArrayType();
                if (at->dimension_count() == 1) {
                    return at->element_type();
                }
                return new (arena_) ArrayType(arena_, at->element_type(), at->dimension_count() - 1,
                                              iterable->source_position());
            } break;
                
            case Type::kType_channel: {
                auto ct = iterable->AsChannelType();
                if (!ct->CanRead()) {
                    Feedback()->Printf(iterable->source_position(), "Channel `%s' can not be read",
                                       iterable->ToString().c_str());
                    return nullptr;
                }
                return ct->element_type();
            } break;
                
            default:
                Feedback()->Printf(iterable->source_position(), "Type `%s' is not iterable",
                                   iterable->ToString().c_str());
                return nullptr;
        }
    }
    
    Statement *GetTypeSpecifiedDefinition(Type *type) {
        switch (type->primary_type()) {
            case Type::kType_class:
                return type->AsClassType()->definition();
            case Type::kType_struct:
                return type->AsStructType()->definition();
            case Type::kType_interface:
                return type->AsInterfaceType()->definition();
            case Type::kType_any:
            case Type::kType_string:
            /*case Type::kType_bool:
            case Type::kType_char:
            case Type::kType_i8:
            case Type::kType_u8:
            case Type::kType_i16:
            case Type::kType_u16:
            case Type::kType_i32:
            case Type::kType_u32:
            case Type::kType_i64:
            case Type::kType_u64:
            case Type::kType_f32:
            case Type::kType_f64:*/ {
                auto symbol = FindGlobal(kLangPackageFullName, Constants::kPrimitiveTypeClassNames[type->primary_type()]);
                DCHECK(symbol.IsFound());
                return symbol.ast;
            }
            default:
                return nullptr;
        }
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
        if (symbol->IsTemplate()) {
            InstantiatingResolver resover(this);
            auto rs = GenericsInstantiating::Instantiate(nullptr, symbol, arena_, error_feedback_, &resover,
                                                         types.size(), &types[0], &symbol);
            if (rs.fail()) {
                return nullptr;
            }
            if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
                return nullptr;
            }
        }

        ast->set_instantiated(symbol);
        if (Definition::Is(symbol)) {
            auto def = down_cast<Definition>(symbol);
            if (def->package() == location_->NearlyPackageScope()->pkg()) {
                deps_->AddBackward(arena_, def->name()->ToSlice(), symbol);
            }
        } else {
            auto decl = down_cast<Declaration>(symbol);
            if (decl->package() == location_->NearlyPackageScope()->pkg()) {
                deps_->AddBackward(arena_, decl->Identifier()->ToSlice(), symbol);
            }
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
            symbol = file_scope->FindLocalSymbol(MakeFullInstantiatingName(name->ToSlice(),
                                                                           static_cast<int>(types.size()), &types[0]));
            if (!symbol) {
                symbol = file_scope->FindLocalSymbol(name->ToSlice());
            }
        }
        if (!symbol || (kind != Node::kMaxKinds && symbol->kind() != kind)) {
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
            auto inst_name = MakeFullInstantiatingName(name->ToSlice(), static_cast<int>(types.size()), &types[0]);
            symbol = file_scope->FindExportSymbol(prefix->name()->ToSlice(), inst_name);
            if (!symbol) {
                symbol = file_scope->FindExportSymbol(prefix->name()->ToSlice(), name->ToSlice());
            }
        }
        if (!symbol || (kind != Node::kMaxKinds && symbol->kind() != kind)) {
            Feedback()->Printf(ast->source_position(), "%s: %s.%s not found", info, prefix->name()->data(),
                               name->data());
            return nullptr;
        }
        if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
            return nullptr;
        }
        return symbol;
    }
    
    int ReduceDependencySymbolIfNeeded(const char *name, Statement *node, Statement *ast) {
        if (ProcessDependencySymbolIfNeeded(ast) < 0) {
            return -1;
        }
        
        AddBackwardDepsIfNeeded(name, ast);
        switch (ast->kind()) {
            case Node::kFunctionDeclaration:
                return Returning(ast->AsFunctionDeclaration()->prototype());
            case Node::kObjectDeclaration:
                return Returning(ast->AsObjectDeclaration()->Type());
            case Node::kVariableDeclaration:
                DCHECK(ast->AsVariableDeclaration()->ItemSize() == 1);
                return Returning(ast->AsVariableDeclaration()->Type());
            case Node::kClassDefinition:
            case Node::kStructDefinition:
            case Node::kEnumDefinition:
                break;
            default: {
                if (auto var = down_cast<VariableDeclaration::Item>(ast)) {
                    DCHECK(var->type() != nullptr);
                    auto ty = LinkType(var->type());
                    if (!ty) {
                        return -1;
                    }
                    var->set_type(ty);
                    return Returning(var->type());
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
                
                if (!RecursionEnterAndTest(ast)) {
                    Feedback()->Printf(ast->source_position(), "Recursived dependency symbol");
                    return -1;
                }
                if (owns != current_file_scope->file_unit()) {
                    //printd("nested reduce: %s", down_cast<FileUnit>(owns)->file_name()->data());
                    PackageScope *external_scope = nullptr;
                    FileUnitScope *scope = nullptr;
                    if (pkg != current_pkg_scope->pkg()) {
                        external_scope = EnsurePackageScope(pkg);
                        external_scope->Enter();
                        scope = DCHECK_NOTNULL(external_scope->FindFileUnitScopeOrNull(owns));
                    } else {
                        scope = DCHECK_NOTNULL(current_pkg_scope->FindFileUnitScopeOrNull(owns));
                    }
                    DCHECK(scope != current_file_scope);
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
                RecursionExit(ast);
            }
        }
        return 0;
    }
    
    int Returning(Type *type) {
        results_.push(DCHECK_NOTNULL(type));
        return 1;
    }
    
    int Returning(const std::vector<Type *> &types) {
        for (auto ty : types) {
            Returning(ty);
        }
        return static_cast<int>(types.size());
    }
    
    int ReduceReturningAtLeastOne(AstNode *node, Type **receiver) {
        const int nrets = node->Accept(this);
        if (fail() || nrets < 0) {
            return -1;
        }
        *receiver = results_.top();
        int i = nrets - 1;
        while (i--) { results_.pop(); }
        return nrets;
    }
    
    int ReduceReturningOnlyOne(AstNode *node, Type **receiver, const char *message = "More than one values") {
        const int nrets = node->Accept(this);
        if (fail() || nrets < 0) {
            return -1;
        }
        if (nrets != 1) {
            Feedback()->Printf(node->source_position(), "%s, actual is %d", message, nrets);
            return -1;
        }
        *receiver = results_.top();
        int i = nrets - 1;
        while (i--) { results_.pop(); }
        return nrets;
    }
    
    int Reduce(AstNode *node, std::vector<Type *> *receiver = nullptr) {
        const int nrets = node->Accept(this);
        if (fail() || nrets < 0) {
            return -1;
        }
        size_t origin_level = 0;
        if (receiver) {
            origin_level = receiver->size();
            receiver->resize(origin_level + nrets);
        }
        int i = nrets;
        while (i--) {
            if (receiver) {
                (*receiver)[origin_level + i] = results_.top();
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
            auto full_name = MakeFullInstantiatingName(name->name()->ToSlice(),
                                                       static_cast<int>(owns->generic_args_size()),
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
            Feedback()->Printf(name->source_position(), "Symbol: `%s' not found!", name->ToString().c_str());
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
            if (ProcessDependencySymbolIfNeeded(symbol) < 0) {
                return nullptr;
            }
        }

        if (!name->prefix_name()) {
            AddBackwardDepsIfNeeded(name->name()->ToSlice(), symbol);
        }
        switch (symbol->kind()) {
            case Node::kClassDefinition:
                return new (arena_) ClassType(arena_, symbol->AsClassDefinition(), name->source_position());
            case Node::kStructDefinition:
                return new (arena_) StructType(arena_, symbol->AsStructDefinition(), name->source_position());
            case Node::kEnumDefinition:
                return new (arena_) EnumType(arena_, symbol->AsEnumDefinition(), name->source_position());
            case Node::kInterfaceDefinition:
                return new (arena_) InterfaceType(arena_, symbol->AsInterfaceDefinition(), name->source_position());
            case Node::kObjectDeclaration:
            default:
                Feedback()->Printf(name->source_position(), "%s is not a type", name->ToString().c_str());
                break;
        }
        return nullptr;
    }
    
    void AddBackwardDepsIfNeeded(std::string_view name, Statement *ast) {
        auto [owns, _] = ast->Owns(true/*force*/);
        auto pkg = ast->Pack(true/*force*/);
        if (owns && owns->IsFileUnit()) { // Is global symbol
            if (DCHECK_NOTNULL(deps_)->pkg() == pkg) {
                deps_->AddBackward(arena_, name, ast);
            }
        }
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
        //printd("insert global: %s", symbol.symbol->data());
        
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
            //DCHECK(expr->IsDot());
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
    
    Type *StringTy() {
        if (!string_) {
            string_ = new (arena_) Type(arena_, Type::kType_string, {0,0});
        }
        return string_;
    }
    
    Type *I32() {
        if (!i32_) {
            i32_ = new (arena_) Type(arena_, Type::kType_i32, {0,0});
        }
        return i32_;
    }
    
    Type *Bool() {
        if (!bool_) {
            bool_ = new (arena_) Type(arena_, Type::kType_bool, {0,0});
        }
        return bool_;
    }
    
    Type *Any() {
        if (!any_) {
            any_ = new (arena_) Type(arena_, Type::kType_any, {0,0});
        }
        return any_;
    }
    
    Type *Array(Type *element_type, const SourcePosition &source_position = {0,0}) {
        return new (arena_) ArrayType(arena_, element_type, 1, source_position);
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
        DCHECK(iter != package_scopes_.end());
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
    
    String *MakePrototypeSignature(FunctionPrototype *prototype) {
        std::string incoming(prototype->MakeSignature());
        auto iter = prototype_signatures_.find(incoming);
        if (iter != prototype_signatures_.end()) {
            return iter->second;
        }
        auto sign = String::New(arena_, incoming.data(), incoming.size());
        prototype_signatures_[sign->ToSlice()] = sign;
        return sign;
    }
    
    bool RecursionEnterAndTest(Statement *ast) {
        auto iter = recursion_tracing_.find(ast);
        if (iter != recursion_tracing_.end()) {
            return false;
        }
        recursion_tracing_.insert(ast);
        return true;
    }
    
    void RecursionExit(Statement *ast) {
        auto iter = recursion_tracing_.find(ast);
        DCHECK(iter != recursion_tracing_.end());
        recursion_tracing_.erase(iter);
    }
    
    base::Arena *const arena_;
    SyntaxFeedback *const error_feedback_;
    PrimitiveTypesProperties *const props_;
    NamespaceScope *location_ = nullptr;
    SymbolDepsScope *deps_ = nullptr;
    Package *entry_;
    std::set<Package *> track_;
    std::set<Statement *> recursion_tracing_; // For
    base::Status status_;
    std::unordered_map<std::string_view, GlobalSymbol> global_symbols_;
    std::map<Package *, PackageScope *> package_scopes_;
    std::stack<Type *> results_;
    std::unordered_map<std::string_view, String *> prototype_signatures_;
    Type *unit_ = nullptr;
    Type *string_ = nullptr;
    Type *i32_ = nullptr;
    Type *any_ = nullptr;
    Type *bool_ = nullptr;
}; // class TypeReducingVisitor

TypeReducingVisitor::TypeReducingVisitor(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback)
: arena_(arena)
, error_feedback_(error_feedback)
, props_(arena->Lazy<PrimitiveTypesProperties>())
, entry_(entry) {
}


base::Status Compiler::ReducePackageDependencesType(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback,
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
