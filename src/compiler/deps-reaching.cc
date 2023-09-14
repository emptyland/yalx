#include "compiler/generics-instantiating.h"
#include "compiler/syntax-feedback.h"
#include "compiler/scope.h"
#include "compiler/ast.h"
#include "compiler/constants.h"
#include "compiler/compiler.h"
#include <stack>

namespace yalx {

namespace cpl {

#define REDUCE(ast) if ((ast)->Accept(this) < 0) { return -1; } (void)0
#define REDUCE_TYPE(ty) if (Reduce((ty)) == nullptr) { return -1; } (void)0

class DepsReachingVisitor : public AstVisitor {
public:
    DepsReachingVisitor(Package *entry, base::Arena *arena, SyntaxFeedback *error_feedback)
    : arena_(arena)
    , error_feedback_(error_feedback)
    , entry_(entry) {}
    
    base::Status Reduce() {
        using std::placeholders::_1;
        
        Recursive(entry_, std::bind(&DepsReachingVisitor::PreparePackage, this, _1));
        if (fail()) {
            return status_;
        }
        return Recursive(entry_, std::bind(&Package::Accept, _1, this));
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
                        assert(any.IsFound());
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
        if (Track(node)) {
            return 0; // Skip if has processed
        }
        NamespaceScope::Keeper<PackageScope> keeper(EnsurePackageScope(node));
        error_feedback_->set_package_name(node->name()->ToString());
        
        for (auto file_scope : keeper.ns()->files()) {
            NamespaceScope::Keeper<FileUnitScope> file_keeper(file_scope);
            error_feedback_->set_file_name(file_scope->file_unit()->file_name()->ToString());
            REDUCE(file_scope->file_unit());
        }
        return 0;
    }
    
    int VisitFileUnit(FileUnit *node) override {
        for (auto ast : node->statements()) {
            auto pkg_scope = location_->NearlyPackageScope();
            if (pkg_scope->Track(ast)) {
                continue;
            }
            REDUCE(ast);
        }
        return 0;
    }
    
    int VisitBlock(Block *node) override {
        for (auto ast : node->statements()) {
            REDUCE(ast);
        }
        return 0;
    }
    
    int VisitList(List *node) override {
        for (auto ast : node->expressions()) {
            REDUCE(ast);
        }
        return 0;
    }
    
    int VisitVariableDeclaration(VariableDeclaration *node) override {
        bool in_global_scope = (node->owns() && node->owns()->IsFileUnit());
        if (in_global_scope) {
            auto pkg_scope = location_->NearlyPackageScope();
            //pkg_scope->pkg()->FindOrInsertDeps(, <#Statement *ast#>)
            for (int i = 0; i < node->ItemSize(); i++) {
                auto item = node->AtItem(i);
                auto deps = pkg_scope->pkg()->FindOrInsertDeps(arena_, item->Identifier()->ToSlice(), node);
                USE(deps);
                // TODO:
                
                if (item->Type()) {
                    REDUCE_TYPE(item->Type());
                }
            }
            for (auto expr : node->initilaizers()) {
                REDUCE(expr);
            }
            
        } else {
            for (int i = 0; i < node->ItemSize(); i++) {
                auto item = node->AtItem(i);
                location_->FindOrInsertSymbol(item->Identifier()->ToSlice(), item);
            }
            for (auto expr : node->initilaizers()) {
                REDUCE(expr);
            }
        }
        return 0;
    }
    
    Type *Reduce(Type *type) {
        using std::placeholders::_1;
        using std::placeholders::_2;
        return type->Link(std::bind(&DepsReachingVisitor::TypeLinkage, this, _1, _2));
    }
    
    Type *TypeLinkage(const Symbol *name, Type *owns) {
        DCHECK(0);
        return nullptr;
    }
    
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
    
    GlobalSymbol FindOrInsertGlobal(Package *owns, std::string_view name, Statement *ast) {
        std::string full_name = owns->path()->ToString();
        full_name.append(":").append(owns->name()->ToString()).append(".").append(name.data(), name.size());
        
        auto iter = global_symbols_.find(full_name);
        if (iter != global_symbols_.end()) {
            return iter->second;
        }
        GlobalSymbol symbol = {String::New(arena_, full_name), ast, owns};
        global_symbols_[symbol.symbol->ToSlice()] = symbol;
        //printd("insert global: %s", symbol.symbol->data());
        
        return GlobalSymbol::NotFound();
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
    
    bool fail() const { return status_.fail(); }
    
    base::Arena *const arena_;
    SyntaxFeedback *const error_feedback_;
    NamespaceScope *location_ = nullptr;
    Package *entry_;
    base::Status status_;
    std::set<Package *> track_;
    std::set<Statement *> recursion_tracing_; // For
    std::unordered_map<std::string_view, GlobalSymbol> global_symbols_;
    std::map<Package *, PackageScope *> package_scopes_;
}; // DepsReachingVisitor

} // namespace cpl

} // namespace yalx
