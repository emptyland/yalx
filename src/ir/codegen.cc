#include "ir/codegen.h"
#include "ir/scope.h"
#include "ir/operators-factory.h"
#include "ir/operator.h"
#include "ir/node.h"
#include "ir/metadata.h"
#include "ir/constants.h"
#include "compiler/ast.h"
#include "compiler/syntax-feedback.h"
#include "compiler/constants.h"
#include "base/checking.h"
#include "base/format.h"
#include <stack>
#include <set>

namespace yalx {

namespace ir {

#define REDUCE(stmt) \
if (auto rs = Reduce(stmt); rs < 0) { \
return -1; \
} (void)0

#define CURRENT_SOUCE_POSITION(ast) \
CurrentFileName()->ToSlice(), \
(ast)->source_position(), \
module_->mutable_source_position_table()

class LoopContext final {
public:
    LoopContext(LoopContext **location, BasicBlock *loop_entry, BasicBlock *loop_exit)
    : location_(location)
    , prev_(nullptr)
    , loop_entry_(loop_entry)
    , loop_exit_(loop_exit) {
        Enter();
    }
    
    ~LoopContext() { Exit(); }
    
    DEF_PTR_GETTER(BasicBlock, loop_entry);
    DEF_PTR_GETTER(BasicBlock, loop_exit);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(LoopContext);
private:
    void Enter() {
        assert(location_ != nullptr);
        prev_ = *location_;
        assert(*location_ != this);
        *location_ = this;
    }
    
    void Exit() {
        assert(*location_ == this);
        *location_ = prev_;
    }
    
    LoopContext **location_;
    LoopContext *prev_;
    BasicBlock *loop_entry_;
    BasicBlock *loop_exit_;
}; // class LoopContext

class FunContext final {
public:
    FunContext(FunContext **location, Function *fun, BasicBlock *current_block)
    : location_(DCHECK_NOTNULL(location))
    , prev_(nullptr)
    , fun_(fun)
    , current_block_(current_block) {
        Enter();
    }
    
    ~FunContext() { Exit(); }
    
    DEF_PTR_GETTER(Function, fun);
    DEF_PTR_PROP_RW(BasicBlock, current_block);
    DEF_PTR_GETTER(LoopContext, current_loop);
    
    LoopContext **loop_location() { return &current_loop_; }
    
private:
    void Enter() {
        assert(location_ != nullptr);
        prev_ = *location_;
        assert(*location_ != this);
        *location_ = this;
    }
    
    void Exit() {
        assert(*location_ == this);
        *location_ = prev_;
    }
    
    FunContext **location_;
    FunContext *prev_;
    Function *fun_;
    BasicBlock *current_block_;
    LoopContext *current_loop_ = nullptr;
}; // class Emitting

class IRGeneratorAstVisitor : public cpl::AstVisitor {
public:
    IRGeneratorAstVisitor(IntermediateRepresentationGenerator *owns): owns_(owns) {}
    
    int VisitPackage(cpl::Package *node) override {
        assert(module_ == nullptr);
        
        auto name = node->path()->ToString().append(":").append(node->name()->ToString());
        module_ = DCHECK_NOTNULL(owns_->AssertedGetModule(name));
        init_fun_ = DCHECK_NOTNULL(module_->FindFunOrNull(cpl::kModuleInitFunName));
        init_blk_ = DCHECK_NOTNULL(init_fun_->entry());
        FunContext emitter(&emitting_, init_fun_, init_blk_);
        
        auto scope = owns_->AssertedGetPackageScope(node);
        scope->Enter(&location_);
        feedback()->set_package_name(name);
        
        EmitCallingModuleDependentInitializers(node);
        
        //        for (auto file_unit : node->source_files()) {
        //            feedback()->set_file_name(file_unit->file_name()->ToString());
        //            if (auto rs = file_unit->Accept(this); rs < 0 || fail()) {
        //                scope->Exit();
        //                return -1;
        //            }
        //        }
        for (auto [_, deps] : node->deps_of_symbols()) {
            std::set<cpl::Statement *> recursion_tracing;
            if (RecursiveReduceSymbols(scope, deps, &recursion_tracing) < 0) {
                scope->Exit();
                return -1;
            }
        }
        scope->Exit();
        
        // Finalize ret instr
        b()->NewNode(SourcePosition::Unknown(), Types::Void, ops()->Ret(0));
        init_fun_->UpdateIdsOfBlocks();
        return 0;
    }
    
    int RecursiveReduceSymbols(PackageScope *pkg_scope, cpl::SymbolDepsNode *deps,
                               std::set<cpl::Statement *> *recursion_tracing) {
        recursion_tracing->insert(deps->ast());
        
        for (auto backward : deps->backwards()) {
            if (recursion_tracing->find(backward->ast()) != recursion_tracing->end()) {
                continue;
            }
            if (RecursiveReduceSymbols(pkg_scope, backward, recursion_tracing) < 0) {
                return -1;
            }
        }
        
        if (pkg_scope->Track(deps->ast())) {
            return 0;
        }
        auto [owns, _] = deps->ast()->Owns(true/*force*/);
        //auto pkg = deps->ast()->Pack(true/*force*/);
        assert(pkg_scope->pkg() == deps->ast()->Pack(true/*force*/));
        NamespaceScope::Keeper<FileUnitScope> holder(pkg_scope->FindFileUnitScopeOrNull(owns));
        return Reduce(deps->ast());
    }
    
    int VisitFileUnit(cpl::FileUnit *node) override {
        auto pkg_scope = location_->NearlyPackageScope();
        auto file_scope = DCHECK_NOTNULL(pkg_scope->FindFileUnitScopeOrNull(node));
        NamespaceScope::Keeper<FileUnitScope> holder(file_scope);
        for (auto stmt : node->statements()) {
            if (pkg_scope->Track(stmt)) {
                continue;
            }
            REDUCE(stmt);
        }
        return 0;
    }
    
    int VisitClassDefinition(cpl::ClassDefinition *node) override {
        return GenerateStructureModel(node, [this, node](StructureModel *clazz) {
            for (auto concept: node->concepts()) {
                auto ty = BuildType(concept);
                assert(ty.kind() == Type::kValue);
                assert(ty.model()->declaration() == Model::kInterface);
                clazz->mutable_interfaces()->push_back(down_cast<InterfaceModel>(ty.model()));
            }
        });
    }
    
    int VisitStructDefinition(cpl::StructDefinition *node) override {
        return GenerateStructureModel(node, [](StructureModel *){});
    }
    
    template<class T> int GenerateStructureModel(T *node, std::function<void(StructureModel *)> &&fixup) {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        
        auto clazz = AssertedGetUdt<StructureModel>(node->FullName());
        if (node->base_of()) {
            auto base = AssertedGetUdt<StructureModel>(node->base_of()->FullName());
            clazz->set_base_of(base);
        }
        
        StructureScope scope(&location_, node, clazz);
        scope.InstallAncestorsSymbols();
        
        for (auto ast : node->fields()) {
            Model::Field field {
                ast.declaration->Identifier()->Duplicate(arena()),
                kPublic, // TODO:
                0,
                BuildType(ast.declaration->Type()),
                false,
            };
            auto handle = clazz->InsertField(field);
            location_->PutSymbol(field.name->ToSlice(), Symbol::Had(location_, handle, ast.declaration));
        }
        
        for (auto ast : node->methods()) {
            auto proto = owns_->BuildPrototype(ast->prototype(), clazz);
            auto fun = module_->NewFunction(ToDecoration(ast), ast->name(), clazz, proto);
            Model::Method method {
                .fun = fun,
                .access = kPublic,
                .in_itab = 0,
                .in_vtab = 0,
            };
            auto handle = clazz->InsertMethod(method);
            location_->PutSymbol(method.fun->name()->ToSlice(), Symbol::Had(location_, handle, ast));
        }
        
        for (auto ast : node->methods()) {
            if (ast->decoration() == cpl::FunctionDeclaration::kNative ||
                ast->body() == nullptr) {
                continue;
            }
            auto method = clazz->FindMethod(ast->name()->ToSlice());
            assert(method.has_value());
            if (!GenerateFun(ast, clazz, method->fun)) {
                return -1;
            }
        }
        
        if (node->primary_constructor()) {
            auto ctor = GenerateFun(node->primary_constructor(), clazz);
            if (!ctor) {
                return -1;
            }
            clazz->set_constructor(ctor);
            Model::Method method {
                .fun = ctor,
                .access = kPublic,
                .in_vtab = 0,
                .in_itab = 0,
            };
            clazz->InsertMethod(method);
            
            //printd("ctor: %s", node->primary_constructor()->name()->data());
        }
        
        fixup(clazz);
        clazz->InstallVirtualTables(false/*force*/);
        return Returning(Unit());
    }
    
    int VisitObjectDeclaration(cpl::ObjectDeclaration *node) override {
        SourcePositionTable::Scope ss_root(CURRENT_SOUCE_POSITION(node));
        assert(location_->IsFileUnitScope());
        
        auto full_name = MakeFullName(node->name());
        auto value = owns_->AssertedGetVal(full_name);
        auto op = ops()->StoreGlobal();
        b()->NewNode(ss_root.Position(), Types::Void, op, value, Nil(value->type()));
        return Returning(Unit());
    }
    
    int VisitVariableDeclaration(cpl::VariableDeclaration *node) override {
        SourcePositionTable::Scope ss_root(CURRENT_SOUCE_POSITION(node));
        const bool in_global_scope = (node->owns() && node->owns()->IsFileUnit());
        
        std::vector<Value *> init_vals;
        for (auto ast : node->initilaizers()) {
            if (Reduce(ast, &init_vals) < 0) {
                return -1;
            }
        }
        
        assert(init_vals.size() >= node->ItemSize());
        for (auto i = 0; i < node->ItemSize(); i++) {
            auto ast = node->AtItem(i);
            SourcePositionTable::Scope ss(ast->source_position(), &ss_root);
            if (cpl::Identifier::IsPlaceholder(ast->Identifier())) {
                continue;
            }
            if (!in_global_scope) {
                auto val = Symbol::Val(location_, init_vals[i], b());
                location_->PutSymbol(ast->Identifier()->ToSlice(), val);
                continue;
            }
            
            auto dest = location_->FindSymbol(ast->Identifier()->ToSlice());
            assert(dest.IsFound());
            auto op = ops()->StoreGlobal();
            b()->NewNode(ss.Position(), Types::Void, op, dest.core.value, init_vals[i]);
        }
        
        return Returning(Unit());
    }
    
    void LinkTo(Function *fun, BasicBlock *from, BasicBlock *to) {
        auto term = from->instructions().empty() ? nullptr : from->instructions().back();
        bool is_term = term && term->op()->value() == Operator::kBr && term->op()->control_out() == 1;
        if (is_term) {
            term->SetOutputControl(0, to);
        } else {
            auto op = ops()->Br(0/*value_in*/, 1/*control_out*/);
            from->NewNode(SourcePosition::Unknown(), Types::Void, op, to);
        }
        // [x] -> d -> a -> c
        // d -> a
        printd("link %s -> %s",
               !from->name() ? "[x]" : from->name()->data(),
               !to->name() ? "[x]" : to->name()->data());
        
        fun->MoveToAfterOf(from, to);
        from->LinkTo(to);
    }
    
    int VisitBlock(cpl::Block *node) override {
        std::vector<Value *> values = { Unit() };
        for (auto ast : node->statements()) {
            values.clear();
            if (Reduce(ast, &values) < 0) {
                return -1;
            }
        }
        return Returning(values);
    }
    
    int VisitList(cpl::List *node) override {
        std::vector<Value *> values;
        for (auto ast : node->expressions()) {
            if (Reduce(ast, &values) < 0) {
                return -1;
            }
        }
        assert(!values.empty());
        return Returning(values);
    }
    
    int VisitReturn(cpl::Return *node) override {
        SourcePositionTable::Scope ss(CURRENT_SOUCE_POSITION(node));
        if (node->returnning_vals().empty()) {
            auto ret = b()->NewNode(ss.Position(), Types::Void, ops()->Ret(0));
            return Returning(ret);
        }
        
        std::vector<Value *> values;
        for (auto val : node->returnning_vals()) {
            if (auto rs = Reduce(val, &values); rs < 0) {
                return -1;
            }
        }
        auto ret = b()->NewNodeWithValues(nullptr/*name*/, ss.Position(), Types::Void,
                                          ops()->Ret(static_cast<int>(values.size())),
                                          values);
        return Returning(ret);
    }
    
    int VisitIdentifier(cpl::Identifier *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        
        auto symbol = location_->FindSymbol(node->name()->ToSlice());
        assert(symbol.IsFound());
        switch (symbol.kind) {
            case Symbol::kValue: {
                auto value = symbol.core.value;
                if (value->Is(Operator::kGlobalValue)) {
                    auto op = ops()->LoadGlobal();
                    auto load = b()->NewNode(root_ss.Position(), value->type(), op, value);
                    return Returning(load);
                } else if (value->Is(Operator::kLazyValue)) {
                    auto op = ops()->LazyValue(node->name());
                    auto load = b()->NewNode(root_ss.Position(), value->type(), op, value);
                    return Returning(load);
                } else {
                    return Returning(symbol.core.value);
                }
            } break;
            case Symbol::kHandle: {
                auto handle = symbol.core.handle;
                auto this_val = location_->FindSymbol(cpl::kThisName);
                assert(this_val.IsFound());
                return Returning(EmitLoadField(this_val.core.value, handle, root_ss.Position()));
            } break;
            case Symbol::kFun: {
                auto fun = symbol.core.fun;
                auto op = ops()->Closure(fun);
                auto type = Type::Ref(fun->prototype());
                auto load = b()->NewNode(root_ss.Position(), type, op);
                return Returning(load);
            } break;
            default:
                UNREACHABLE();
                break;
        }
    }
    
    int VisitCalling(cpl::Calling *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        std::vector<Value *> args;
        Handle *handle = nullptr;
        
        Symbol symbol = Symbol::NotFound();
        if (auto ast = node->callee()->AsIdentifier()) {
            symbol = location_->FindSymbol(ast->name()->ToSlice());
        } else if (auto ast = node->callee()->AsDot()) {
            if (auto id = ast->primary()->AsIdentifier()) {
                auto file_scope = DCHECK_NOTNULL(location_->NearlyFileUnitScope());
                symbol = file_scope->FindExportSymbol(id->name()->ToSlice(), ast->field()->ToSlice());
            }
            if (symbol.IsNotFound()) {
                Value *value = nullptr;
                if (ReduceReturningOnlyOne(ast->primary(), &value) < 0) {
                    return -1;
                }
                handle = DCHECK_NOTNULL(value->type().model())->FindMemberOrNull(ast->field()->ToSlice());
                if (handle && handle->IsMethod()) {
                    args.push_back(value);
                } else {
                    handle = nullptr;
                }
            }
        } else if (auto ast = node->callee()->AsInstantiation()) {
            // a.b<...>
            // b<...>
            auto inst = DCHECK_NOTNULL(ast->instantiated());
            std::string full_name;
            if (cpl::Definition::Is(inst)) {
                auto def = static_cast<cpl::Definition *>(inst);
                full_name = def->package()->path()->ToString().append(":").append(def->FullName());
            } else {
                assert(inst->IsFunctionDeclaration());
                auto decl = static_cast<cpl::Declaration *>(inst);
                full_name = decl->package()->path()->ToString().append(":").append(decl->FullName());
            }
            symbol = owns_->AssertedGetSymbol(full_name);
        }
        
        for (auto ast : node->args()) {
            if (Reduce(ast, &args) < 0) {
                return -1;
            }
        }
        if (symbol.kind == Symbol::kHandle) {
            handle = symbol.core.handle;
            auto arg0 = location_->FindSymbol(cpl::kThisName);
            assert(arg0.kind == Symbol::kValue);
            args.insert(args.begin(), arg0.core.value);
        }
        if (handle) { // It's method calling
            auto method = std::get<const Model::Method *>(DCHECK_NOTNULL(args[0]->type().model())->GetMember(handle));
            auto proto = method->fun->prototype();
            auto type = proto->return_type(0);
            auto value_in = static_cast<int>(proto->params_size());
            Operator *op = nullptr;
            assert(args[0]->type().kind() == Type::kValue || args[0]->type().IsReference());
            auto self = DCHECK_NOTNULL(args[0]->type().model());
            if (self->declaration() == Model::kInterface) {
                op = ops()->CallAbstract(handle, 1/*value_out*/, value_in);
            } else if (self->declaration() == Model::kClass) {
                if (down_cast<StructureModel>(self)->In_vtab(handle)) {
                    op = ops()->CallVirtual(handle, 1/*value_out*/, value_in);
                } else {
                    op = ops()->CallHandle(handle, 1/*value_out*/, value_in);
                }
            } else {
                assert(self->declaration() == Model::kStruct);
                op = ops()->CallHandle(handle, 1/*value_out*/, value_in);
            }
            
            std::vector<Value *> results;
            EmitCall(op, proto, std::move(args), &results, root_ss.Position());
            return Returning(results);
        }
        
        if (symbol.IsNotFound()) {
            Value *callee = nullptr;
            if (ReduceReturningOnlyOne(node->callee(), &callee) < 0) {
                return -1;
            }
            auto proto = down_cast<PrototypeModel>(callee->type().model());
            auto type = proto->return_type(0);
            auto value_in = static_cast<int>(proto->params_size());
            auto op = ops()->CallIndirectly(1/*value_out*/, value_in);
            args.insert(args.begin(), callee);
            std::vector<Value *> results;
            EmitCall(op, proto, std::move(args), &results, root_ss.Position());
            return Returning(results);
        }
        
        if (symbol.kind == Symbol::kFun) {
            auto proto = symbol.core.fun->prototype();
            auto value_in = static_cast<int>(proto->params_size());
            auto op = ops()->CallDirectly(symbol.core.fun, 1/*value_out*/, value_in);
            std::vector<Value *> results;
            EmitCall(op, proto, std::move(args), &results, root_ss.Position());
            return Returning(results);
        }
        
        if (symbol.kind == Symbol::kModel) {
            assert(symbol.core.model->declaration() == Model::kClass ||
                   symbol.core.model->declaration() == Model::kStruct);
            
            auto clazz = down_cast<StructureModel>(symbol.core.model);
            
            Operator *op = nullptr;
            Type type = Types::Void;
            if (clazz->constraint() == Model::kVal) {
                op = ops()->StackAlloc(clazz);
                type = Type::Val(clazz);
            } else {
                op = ops()->HeapAlloc(clazz);
                type = Type::Ref(clazz);
            }
            auto ob = b()->NewNode(root_ss.Position(), type, op);
            
            auto proto = clazz->constructor()->prototype();
            auto value_in = static_cast<int>(proto->params_size());
            auto handle = DCHECK_NOTNULL(clazz->FindMemberOrNull(clazz->constructor()->name()->ToSlice()));
            op = ops()->CallHandle(handle, 1, value_in);
            std::vector<Value *> results;
            args.insert(args.begin(), ob);
            EmitCall(op, proto, std::move(args), &results, root_ss.Position());
            return Returning(results);
            
        }
        UNREACHABLE();
    }
    
    int VisitAssignment(cpl::Assignment *node) override {
        std::vector<Value *> rvals;
        for (auto ast : node->rvals()) {
            if (Reduce(ast, &rvals) < 0) {
                return -1;
            }
        }
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        assert(rvals.size() == node->lvals_size());
        for (auto i = 0; i < node->lvals_size(); i++) {
            Symbol symbol = Symbol::NotFound();
            
            auto rval = rvals[i];
            auto lval = node->lval(i);
            SourcePositionTable::Scope ss(lval->source_position(), &root_ss);
            if (auto ast = lval->AsIdentifier()) {
                symbol = location_->FindSymbol(ast->name()->ToSlice());
                assert(symbol.IsFound());
                if (symbol.kind == Symbol::kValue) {
                    if (symbol.owns->IsFileUnitScope()) { // Global var
                        b()->NewNode(ss.Position(), Types::Void, ops()->StoreGlobal(),
                                     symbol.core.value, rval);
                    } else if (ShouldCaptureVal(symbol.owns, symbol.core.value)) {
                        // TODO: Capture Val
                        UNREACHABLE();
                    } else {
                        //printd("update: %s", ast->name()->data());
                        location_->PutSymbol(ast->name()->ToSlice(), Symbol::Val(symbol.owns, rval, b()));
                    }
                } else {
                    assert(symbol.kind == Symbol::kHandle);
                    auto this_symbol = location_->FindSymbol(cpl::kThisName);
                    assert(this_symbol.kind == Symbol::kValue);
                    auto this_replace = EmitStoreField(this_symbol.core.value, rval, symbol.core.handle, ss.Position());
                    location_->PutValue(cpl::kThisName, this_replace);
                }
            } else if (auto ast = lval->AsDot()) {
                if (auto id = ast->primary()->AsIdentifier()) {
                    auto file_scope = location_->NearlyFileUnitScope();
                    symbol = file_scope->FindExportSymbol(id->name()->ToSlice(), ast->field()->ToSlice());
                }
                if (symbol.IsFound()) {
                    assert(symbol.owns->IsFileUnitScope());
                    b()->NewNode(ss.Position(), Types::Void, ops()->StoreGlobal(), rval);
                } else {
                    if (ProcessDotChainAssignment(ast, rval) < 0) {
                        return -1;
                    }
                }
            } else {
                UNREACHABLE();
            }
        }
        return Returning(Unit());
    }
    
    int VisitFunctionDeclaration(cpl::FunctionDeclaration *node) override {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        assert(location_->IsFileUnitScope());
        auto full_name = MakeFullName(node->name());
        if (!GenerateFun(node, nullptr/*owns*/, owns_->AssertedGetFun(full_name))) {
            return -1;
        }
        return Returning(Unit());
    }
    
    int VisitInterfaceDefinition(cpl::InterfaceDefinition *node) override {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        assert(location_->IsFileUnitScope());
        auto full_name = MakeFullName(node->name());
        auto model = AssertedGetUdt<InterfaceModel>(full_name);
        for (auto ast : node->methods()) {
            auto full_name = String::New(arena(), ast->FullName());
            auto stub = AssertedGetUdt<StructureModel>(cpl::kAnyClassFullName);
            auto proto = owns_->BuildPrototype(ast->prototype(), stub);
            auto fun = module_->NewStandaloneFunction(ToDecoration(ast), ast->name()->Duplicate(arena()), full_name, proto);
            model->InsertMethod(fun);
        }
        
        return Returning(Unit());
    }
    
    int VisitIfExpression(cpl::IfExpression *node) override {
        using std::placeholders::_1;
        using std::placeholders::_2;
        
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        BranchScope trunk(&location_, node);
        NamespaceScope::Keeper<BranchScope> root_holder(&trunk);
        if (node->initializer()) {
            REDUCE(node->initializer());
        }
        
        Value *condition = nullptr;
        if (ReduceReturningAtLeastOne(node->condition(), &condition) < 0) {
            return -1;
        }
        //std::vector<Value *> values;
        const int number_of_branchs = 1 + (!node->else_clause() ? 0 : 1);
        std::vector<Value *> values[2];
        BasicBlock *blocks[2] = {nullptr, b()/*origin block*/}; // At most two
        
        //auto fun_scope = location_->NearlyFunctionScope();
        auto fun = emitting_->fun();
        auto then_block = fun->NewBlock(nullptr);
        auto else_block = fun->NewBlock(nullptr);
        auto exit_block = node->else_clause() ? fun->NewBlock(nullptr) : else_block;
        
        auto origin = b();
        if (node->then_clause()) {
            SourcePositionTable::Scope ss(node->then_clause()->source_position(), &root_ss);
            b()->NewNode(ss.Position(), Types::Void, ops()->Br(1/*value_in*/, 2/*control_out*/), condition, then_block,
                         else_block);
            
            auto br = trunk.Branch(node->then_clause());
            b(then_block);
            NamespaceScope::Keeper<BranchScope> holder(br);
            if (Reduce(node->then_clause(), &values[0]) < 0) {
                return -1;
            }
            
            blocks[0] = b();
            b()->NewNode(ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/), exit_block);
        }
        
        if (node->else_clause()) {
            SourcePositionTable::Scope ss(node->then_clause()->source_position(), &root_ss);
            auto br = trunk.Branch(node->then_clause());
            b(else_block);
            NamespaceScope::Keeper<BranchScope> holder(br);
            if (Reduce(node->else_clause(), &values[1]) < 0) {
                return -1;
            }
            
            blocks[1] = b();
            b()->NewNode(ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/), exit_block);
        }
        
        SetAndMakeLast(exit_block);
        trunk.MergeConflicts(std::bind(&IRGeneratorAstVisitor::HandleMergeConflicts, this, _1, _2));
        
        std::vector<Type> types;
        for (auto ast : node->reduced_types()) { types.push_back(BuildType(ast)); }
        
        std::vector<Value *> results;
        if (ReduceValuesOfBranches(types, origin, blocks, values, number_of_branchs, 1/*number_of_branchs_without_else*/,
                                   &results, root_ss.Position()) < 0) {
            return -1;
        }
        return Returning(results);
    }
    
    int VisitWhenExpression(cpl::WhenExpression *node) override {
        using std::placeholders::_1;
        using std::placeholders::_2;
        
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        BranchScope scope(&location_, node);
        NamespaceScope::Keeper<BranchScope> trunk(&scope);
        if (node->initializer()) {
            REDUCE(node->initializer());
        }
        
        Value *dest = nullptr;
        if (node->destination()) {
            if (ReduceReturningOnlyOne(node->destination(), &dest) < 0) {
                return -1;
            }
        }
        
        auto fun = emitting_->fun();
        auto else_block = fun->NewBlock(nullptr);
        auto exit_block = node->else_clause() ? fun->NewBlock(nullptr) : else_block;
        
        const int number_of_branchs = node->case_clauses_size() + (!node->else_clause() ? 0 : 1);
        std::vector<std::vector<Value *>> values(number_of_branchs);
        std::vector<BasicBlock *> blocks(number_of_branchs + 1, nullptr);
        blocks[number_of_branchs] = b(); // Initialize last one
        
        auto origin = b();
        auto case_block = fun->NewBlock(nullptr);
        b()->NewNode(root_ss.Position(), Types::Void, ops()->Br(0, 1), case_block);
        
#define NewNextBlock() (case_clause == node->case_clauses().back()) ? else_block : fun->NewBlock(nullptr)
        int i = 0;
        for (auto case_clause : node->case_clauses()) {
            SourcePositionTable::Scope ss(case_clause->source_position(), &root_ss);
            NamespaceScope::Keeper<BranchScope> br(trunk->Branch(node));
            BasicBlock *next_block = nullptr;
            
            switch (case_clause->pattern()) {
                case cpl::CaseWhenPattern::kBetweenTo: {
                    auto ast = cpl::WhenExpression::BetweenToCase::Cast(case_clause);
                    assert(dest != nullptr);
                    
                    b(case_block);
                    Value *lower = nullptr, *upper = nullptr;
                    if (ReduceReturningOnlyOne(ast->lower(), &lower) < 0 ||
                        ReduceReturningOnlyOne(ast->upper(), &upper) < 0) {
                        return -1;
                    }
                    assert(dest->type().Equals(lower->type()));
                    assert(dest->type().Equals(upper->type()));
                    
                    case_block = fun->NewBlock(nullptr);
                    next_block = NewNextBlock();
                    EmitLessOrLessEquals(dest, lower, !ast->is_close(), next_block, case_block, ast);
                    b(case_block);

                    case_block = fun->NewBlock(nullptr);
                    EmitGreaterOrGreaterEquals(dest, upper, !ast->is_close(), next_block, case_block, ast);
                    b(case_block);
                } break;
                    
                case cpl::CaseWhenPattern::kExpectValues: {
                    auto ast = cpl::WhenExpression::ExpectValuesCase::Cast(case_clause);
                    assert(dest != nullptr);

                    b(case_block);
                    std::vector<Value *> expected_values;
                    for (auto match : ast->match_values()) {
                        if (Reduce(match, &expected_values) < 0) {
                            return -1;
                        }
                    }
                    
                    next_block = NewNextBlock();
                    for (auto match_value : expected_values) {
                        case_block = fun->NewBlock(nullptr);
                        EmitEquals(dest, match_value, case_block, next_block, node);
                        b(case_block);
                    }
                } break;
                    
                case cpl::CaseWhenPattern::kTypeTesting: {
                    auto ast = cpl::WhenExpression::TypeTestingCase::Cast(case_clause);
                    assert(dest != nullptr);
                    
                    b(case_block);
                    auto ty = BuildType(ast->match_type());
                    auto cond = EmitTesting(ty, dest, ss.Position());
                    case_block = fun->NewBlock(nullptr);
                    next_block = NewNextBlock();
                    auto op = ops()->Br(1/*value_in*/, 2/*control_out*/);
                    b()->NewNode(ss.Position(), Types::Void, op, cond, case_block, next_block);
                    
                    b(case_block);
                    auto value = EmitCastingIfNeeded(ty, dest, ss.Position());
                    br->PutValue(ast->name()->name()->ToSlice(), value, b());
                } break;
                    
                case cpl::CaseWhenPattern::kStructMatching: {
                    auto ast = cpl::WhenExpression::StructMatchingCase::Cast(case_clause);
                    assert(dest != nullptr);
                    
                    b(case_block);
                    auto ty = BuildType(ast->match_type());
                    assert(ty.model() != nullptr);
                    auto cond = EmitTesting(ty, dest, ss.Position());
                    case_block = fun->NewBlock(nullptr);
                    next_block = NewNextBlock();
                    auto op = ops()->Br(1/*value_in*/, 2/*control_out*/);
                    b()->NewNode(ss.Position(), Types::Void, op, cond, case_block, next_block);
                    
                    b(case_block);
                    auto value = EmitCastingIfNeeded(ty, dest, ss.Position());
                    for (auto expected : ast->expecteds()) {
                        SourcePositionTable::Scope ss1(expected->source_position(), &ss);
                        auto had = DCHECK_NOTNULL(ty.model()->FindMemberOrNull(expected->name()->ToSlice()));
                        auto val = EmitLoadField(value, had, ss1.Position());
                        br->PutValue(expected->name()->ToSlice(), val, b());
                    }
                } break;
                default:
                    UNREACHABLE();
                    break;
            }
            blocks[i] = case_block;
            
            SourcePositionTable::Scope ss1(case_clause->then_clause()->source_position(), &ss);
            if (Reduce(case_clause->then_clause(), &values[i]) < 0) {
                return -1;
            }
            b()->NewNode(ss1.Position(), Types::Void, ops()->Br(0, 1/*control_out*/), exit_block);
            
            case_block = !next_block ? fun->NewBlock(nullptr) : next_block;
            SetAndMakeLast(case_block);
            i++;
        }
#undef NewNextBlock
        if (node->else_clause()) {
            SourcePositionTable::Scope ss(node->else_clause()->source_position(), &root_ss);
            NamespaceScope::Keeper<BranchScope> br(trunk->Branch(node));
            b(else_block);
            if (Reduce(node->else_clause(), &values.back()) < 0) {
                return -1;
            }
            b()->NewNode(ss.Position(), Types::Void, ops()->Br(0, 1/*control_out*/), exit_block);
            blocks[number_of_branchs - 1] = else_block;
        }
        
        SetAndMakeLast(exit_block);
        trunk->MergeConflicts(std::bind(&IRGeneratorAstVisitor::HandleMergeConflicts, this, _1, _2));
        
        std::vector<Type> types;
        for (auto ast : node->reduced_types()) { types.push_back(BuildType(ast)); }
        
        std::vector<Value *> results;
        if (ReduceValuesOfBranches(types, origin, &blocks[0], &values[0], number_of_branchs,
                                   node->case_clauses_size(), &results, root_ss.Position()) < 0) {
            return -1;
        }
        return Returning(results);
    }
    
    int VisitWhileLoop(cpl::WhileLoop *node) override {
        return GenerateConditionLoop(node, true/*while_or_unless*/);
    }
    
    int VisitUnlessLoop(cpl::UnlessLoop *node) override {
        return GenerateConditionLoop(node, false/*while_or_unless*/);
    }
    
    int GenerateConditionLoop(cpl::ConditionLoop *ast, bool while_or_unless) {
        using std::placeholders::_1;
        using std::placeholders::_2;

        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        BranchScope scope(&location_, ast);
        NamespaceScope::Keeper<BranchScope> trunk(&scope);
        if (ast->initializer()) {
            REDUCE(ast->initializer());
        }
        
        auto fun = emitting_->fun();
        auto cond_block = fun->NewBlock(nullptr/*cond*/);
        b()->NewNode(root_ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/), cond_block);
        b(cond_block);
        Value *cond = nullptr;
        if (ReduceReturningOnlyOne(ast->condition(), &cond) < 0) {
            return -1;
        }

        auto loop_block = fun->NewBlock(nullptr/*loop*/);
        auto exit_block = fun->NewBlock(nullptr/*exit*/);
        if (ast->test_first()) {
            LoopContext loop_scope(emitting_->loop_location(), cond_block, exit_block);
            {
                SourcePositionTable::Scope ss(ast->condition()->source_position(), &root_ss);
                auto op = ops()->Br(1/*value_in*/, 2/*control_out*/);
                if (while_or_unless) {
                    b()->NewNode(ss.Position(), Types::Void, op, cond, loop_block, exit_block);
                } else {
                    b()->NewNode(ss.Position(), Types::Void, op, cond, exit_block, loop_block);
                }
            }

            if (ast->body()) {
                SourcePositionTable::Scope ss(ast->body()->source_position(), &root_ss);
                b(loop_block);
                NamespaceScope::Keeper<BranchScope> br(trunk->Branch(ast->body()));
                REDUCE(ast->body());
                b()->NewNode(ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/), cond_block);
            }
            std::map<Value *, Value *> phi_nodes;
            trunk->MergeConflicts(std::bind(&IRGeneratorAstVisitor::HandleMergeConflictsV2,
                                            this, _1, _2, cond_block, &phi_nodes));
            fun->ReplaceUsers(phi_nodes, cond_block, b());
            SetAndMakeLast(exit_block);
        } else {
            LoopContext loop_scope(emitting_->loop_location(), loop_block, exit_block);
            
            if (ast->body()) {
                SourcePositionTable::Scope ss(ast->body()->source_position(), &root_ss);
                b(loop_block);
                NamespaceScope::Keeper<BranchScope> br(trunk->Branch(ast->body()));
                REDUCE(ast->body());
                b()->NewNode(ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/), cond_block);
            }
            
            {
                SourcePositionTable::Scope ss(ast->condition()->source_position(), &root_ss);
                SetAndMakeLast(cond_block);
                auto op = ops()->Br(1/*value_in*/, 2/*control_out*/);
                if (while_or_unless) {
                    b()->NewNode(ss.Position(), Types::Void, op, cond, loop_block, exit_block);
                } else {
                    b()->NewNode(ss.Position(), Types::Void, op, cond, exit_block, loop_block);
                }
            }
            std::map<Value *, Value *> phi_nodes;
            trunk->MergeConflicts(std::bind(&IRGeneratorAstVisitor::HandleMergeConflictsV2,
                                            this, _1, _2, cond_block, &phi_nodes));
            fun->ReplaceUsers(phi_nodes, loop_block, b());
            SetAndMakeLast(exit_block);
        }
        
        return Returning(Unit());
    }
    
    int VisitForeachLoop(cpl::ForeachLoop *node) override {
        
        switch (node->iteration()) {
            case cpl::ForeachLoop::kIterator:
                UNREACHABLE();
                break;
                
            case cpl::ForeachLoop::kOpenBound:
            case cpl::ForeachLoop::kCloseBound:
                if (GenerateForeachRange(node) < 0) {
                    return -1;
                }
                break;
                
            default:
                break;
        }
        return Returning(Unit());
    }
    
    int GenerateForeachRange(cpl::ForeachLoop *ast) {
        using std::placeholders::_1;
        using std::placeholders::_2;

        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        BranchScope scope(&location_, ast);
        NamespaceScope::Keeper<BranchScope> trunk(&scope);
    
        Value *upper = nullptr, *lower = nullptr;
        if (ReduceReturningOnlyOne(ast->range().lower, &lower) < 0 ||
            ReduceReturningOnlyOne(ast->range().upper, &upper) < 0) {
            return -1;
        }
        // Record iterative destination var first
        trunk->PutValue(ast->iterative_destination()->name()->ToSlice(), lower, b());
        auto iter = lower;
        
        auto fun = emitting_->fun();
        auto cond_block = fun->NewBlock(nullptr);
        auto loop_block = fun->NewBlock(nullptr);
        auto exit_block = fun->NewBlock(nullptr);
        LoopContext loop_scope(emitting_->loop_location(), cond_block, exit_block);
        b()->NewNode(root_ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/), cond_block);
        b(cond_block);
        
        assert(upper->type().IsIntegral() && lower->type().IsIntegral());
        Operator *op = nullptr;
        if (upper->type().IsSigned()) {
            op = ops()->ICmp(ast->range().close ? ICondition::slt : ICondition::sle);
        } else {
            op = ops()->ICmp(ast->range().close ? ICondition::ult : ICondition::ule);
        }
        auto cond = b()->NewNode(root_ss.Position(), Types::UInt8, op, iter, upper);
        op = ops()->Br(1/*value_in*/, 2/*control_out*/);
        b()->NewNode(root_ss.Position(), Types::Void, op, cond, loop_block, exit_block);
        
        if (ast->body()) {
            SourcePositionTable::Scope ss(ast->body()->source_position(), &root_ss);
            b(loop_block);
            NamespaceScope::Keeper<BranchScope> br(trunk->Branch(ast->body()));
            REDUCE(ast->body());
            
            // i = i + 1
            auto one = Value::New0(arena(), ss.Position(), iter->type(), ops()->I32Constant(1));
            iter = b()->NewNode(ss.Position(), iter->type(), ops()->Add(), iter, one);
            location_->PutSymbol(ast->iterative_destination()->name()->ToSlice(), Symbol::Val(trunk.ns(), iter, b()));
            b()->NewNode(ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/), cond_block);
        }
        
        std::map<Value *, Value *> phi_nodes;
        trunk->MergeConflicts(std::bind(&IRGeneratorAstVisitor::HandleMergeConflictsV2,
                                        this, _1, _2, cond_block, &phi_nodes));
        fun->ReplaceUsers(phi_nodes, cond_block, b());
        SetAndMakeLast(exit_block);
        return 0;
    }
    
    int VisitBreak(cpl::Break *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        auto op = ops()->Br(0/*value_in*/, 1/*control_out*/);
        b()->NewNode(root_ss.Position(), Types::Void, op, loop()->loop_exit());
        return Returning(Unit());
    }

    int VisitContinue(cpl::Continue *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        auto op = ops()->Br(0/*value_in*/, 1/*control_out*/);
        b()->NewNode(root_ss.Position(), Types::Void, op, loop()->loop_entry());
        return Returning(Unit());
    }
    
    int VisitThrow(cpl::Throw *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        Value *value = nullptr;
        if (ReduceReturningOnlyOne(node->throwing_val(), &value) < 0) {
            return -1;
        }
        assert(value->type().kind() == Type::kReference);
        auto op = ops()->CallRuntime(0, 1/*value_in*/, RuntimeLib::Raise); // Never return
        b()->NewNode(root_ss.Position(), Types::Void, op, value);
        EmitUnreachable();
        return Returning(Unit());
    }
    
    int VisitCasting(cpl::Casting *node) override {
        Value *source = nullptr;
        if (ReduceReturningOnlyOne(node->source(), &source) < 0) {
            return -1;
        }
        auto type = BuildType(node->destination());
        SourcePositionTable::Scope ss(CURRENT_SOUCE_POSITION(node));
        auto dest = EmitCastingIfNeeded(type, source, ss.Position());
        return Returning(dest);
    }
    
    int VisitTesting(cpl::Testing *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        Value *value = nullptr;
        if (ReduceReturningOnlyOne(node->source(), &value) < 0) {
            return -1;
        }
        auto dest_ty = BuildType(node->destination());
        return Returning(EmitTesting(dest_ty, value, root_ss.Position()));
    }
    
    Value *EmitTesting(Type dest_ty, Value *value, SourcePosition source_position) {
        Value *result = nullptr;
        auto src_ty = value->type();
        if (src_ty.IsNumber() && dest_ty.IsNumber()) {
            return Bool(true);
        } else if (src_ty.IsReference() && dest_ty.IsReference()) {
            if (src_ty.model() && dest_ty.model()) {
                if (src_ty.model()->IsBaseOf(dest_ty.model())) {
                    return Bool(true);
                }
            }
            auto op = ops()->IsInstanceOf(dest_ty.model());
            result = b()->NewNode(source_position, Types::UInt8, op, value);
        } else if (value->type().kind() == Type::kValue && dest_ty.kind() == Type::kValue) {
            if (value->type().model() && dest_ty.model()) {
                if (value->type().model()->IsBaseOf(dest_ty.model())) {
                    return Bool(true);
                }
            }
            auto op = ops()->IsInstanceOf(dest_ty.model());
            result = b()->NewNode(source_position, Types::UInt8, op, value);
        } else {
            UNREACHABLE();
        }

        return DCHECK_NOTNULL(result);
    }
    
    int VisitAnnotationDefinition(cpl::AnnotationDefinition *node) override { UNREACHABLE(); }
    int VisitAnnotationDeclaration(cpl::AnnotationDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotation(cpl::Annotation *node) override { UNREACHABLE(); }
    
    int VisitRunCoroutine(cpl::RunCoroutine *node) override { UNREACHABLE(); }
    int VisitStringTemplate(cpl::StringTemplate *node) override { UNREACHABLE(); }
    int VisitInstantiation(cpl::Instantiation *node) override { UNREACHABLE(); }
    int VisitOr(cpl::Or *node) override { UNREACHABLE(); }
    int VisitAnd(cpl::And *node) override { UNREACHABLE(); }
    
    int VisitDot(cpl::Dot *node) override { UNREACHABLE(); }
    int VisitNot(cpl::Not *node) override { UNREACHABLE(); }
    
    int VisitRecv(cpl::Recv *node) override { UNREACHABLE(); }
    int VisitSend(cpl::Send *node) override { UNREACHABLE(); }
    
    int VisitNegative(cpl::Negative *node) override { UNREACHABLE(); }
    
    int VisitIndexedGet(cpl::IndexedGet *node) override { UNREACHABLE(); }
    
    int VisitLambdaLiteral(cpl::LambdaLiteral *node) override { UNREACHABLE(); }
    int VisitArrayInitializer(cpl::ArrayInitializer *node) override { UNREACHABLE(); }
    int VisitTryCatchExpression(cpl::TryCatchExpression *node) override { UNREACHABLE(); }
    int VisitAssertedGet(cpl::AssertedGet *node) override { UNREACHABLE(); }
    int VisitChannelInitializer(cpl::ChannelInitializer *node) override { UNREACHABLE(); }
    
    int VisitAdd(cpl::Add *node) override {
        Operator *candidate[3] = {
            ops()->Add(),
            ops()->Add(),
            ops()->FAdd(),
        };
        return EmitArithBinaryExpression(node, candidate);
    }
    
    int VisitMod(cpl::Mod *node) override {
        Operator *candidate[3] = {
            ops()->SRem(),
            ops()->URem(),
            ops()->FRem(),
        };
        return EmitArithBinaryExpression(node, candidate);
    }
    
    int VisitDiv(cpl::Div *node) override {
        Operator *candidate[3] = {
            ops()->SDiv(),
            ops()->UDiv(),
            ops()->FDiv(),
        };
        return EmitArithBinaryExpression(node, candidate);
    }
    
    int VisitMul(cpl::Mul *node) override {
        Operator *candidate[3] = {
            ops()->Mul(),
            ops()->UMul(),
            ops()->FMul(),
        };
        return EmitArithBinaryExpression(node, candidate);
    }

    int VisitSub(cpl::Sub *node) override {
        Operator *candidate[3] = {
            ops()->Sub(),
            ops()->Sub(),
            ops()->FSub(),
        };
        return EmitArithBinaryExpression(node, candidate);
    }
    
    // candidate[0] = Signed
    // candidate[1] = Unsigned
    // candidate[2] = Float
    int EmitArithBinaryExpression(cpl::BinaryExpression *ast, Operator *candidate[3]) {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        Value *lhs = nullptr, *rhs = nullptr;
        if (ReduceReturningOnlyOne(ast->lhs(), &lhs) < 0 || ReduceReturningOnlyOne(ast->rhs(), &rhs) < 0) {
            return -1;
        }
        assert(lhs->type().IsNumber());
        assert(rhs->type().IsNumber());
        
        Operator *op = nullptr;
        if (lhs->type().IsIntegral() && lhs->type().IsUnsigned() &&
            rhs->type().IsIntegral() && rhs->type().IsUnsigned()) {
            op = candidate[1];
        } else if (lhs->type().IsFloating() && rhs->type().IsFloating()) {
            op = candidate[2];
        } else {
            assert(lhs->type().IsIntegral() && lhs->type().IsSigned());
            assert(rhs->type().IsIntegral() && rhs->type().IsSigned());
            op = candidate[0];
        }
        return Returning(b()->NewNode(root_ss.Position(), lhs->type(), op, lhs, rhs));
    }
    
    int VisitBitwiseNegative(cpl::BitwiseNegative *node) override { UNREACHABLE(); }
    
    int VisitBitwiseOr(cpl::BitwiseOr *node) override {
        Operator *candidate[2] = {
            ops()->Or(),
            ops()->Or()
        };
        return EmitBitwiseBinaryExpression(node, candidate);
    }
    
    int VisitBitwiseAnd(cpl::BitwiseAnd *node) override {
        Operator *candidate[2] = {
            ops()->And(),
            ops()->And()
        };
        return EmitBitwiseBinaryExpression(node, candidate);
    }
    
    int VisitBitwiseXor(cpl::BitwiseXor *node) override {
        Operator *candidate[2] = {
            ops()->Xor(),
            ops()->Xor()
        };
        return EmitBitwiseBinaryExpression(node, candidate);
    }
    
    int VisitBitwiseShl(cpl::BitwiseShl *node) override {
        Operator *candidate[2] = {
            ops()->Shl(), // Singed
            ops()->Shl()  // Unsigned
        };
        return EmitBitwiseBinaryExpression(node, candidate);
    }
    
    int VisitBitwiseShr(cpl::BitwiseShr *node) override {
        Operator *candidate[2] = {
            ops()->AShr(), // Singed
            ops()->LShr()  // Unsigned
        };
        return EmitBitwiseBinaryExpression(node, candidate);
    }
    
    int EmitBitwiseBinaryExpression(cpl::BinaryExpression *ast, Operator *candidate[2]) {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        Value *lhs = nullptr, *rhs = nullptr;
        if (ReduceReturningOnlyOne(ast->lhs(), &lhs) < 0 || ReduceReturningOnlyOne(ast->rhs(), &rhs) < 0) {
            return -1;
        }
        assert(lhs->type().IsIntegral());
        assert(rhs->type().IsIntegral());
        
        Operator *op = nullptr;
        if (lhs->type().IsSigned()) {
            op = candidate[0];
        } else {
            op = candidate[1];
        }
        return Returning(b()->NewNode(root_ss.Position(), lhs->type(), op, lhs, rhs));
    }
    
    int VisitEqual(cpl::Equal *node) override {
        Operator *candidate[] = {
            ops()->FCmp(FCondition::ueq),
            ops()->ICmp(ICondition::eq),
        };
        return EmitComparsion(node, RuntimeLib::StringEQ, candidate, arraysize(candidate));
    }
    
    int VisitNotEqual(cpl::NotEqual *node) override {
        Operator *candidate[] = {
            ops()->FCmp(FCondition::une),
            ops()->ICmp(ICondition::ne),
        };
        return EmitComparsion(node, RuntimeLib::StringNE, candidate, arraysize(candidate));
    }
    
    int VisitLess(cpl::Less *node) override {
        Operator *candidate[3] = {
            ops()->FCmp(FCondition::ult),
            ops()->ICmp(ICondition::slt),
            ops()->ICmp(ICondition::ult),
        };
        return EmitComparsion(node, RuntimeLib::StringLT, candidate, arraysize(candidate));
    }
    
    int VisitLessEqual(cpl::LessEqual *node) override {
        Operator *candidate[3] = {
            ops()->FCmp(FCondition::ule),
            ops()->ICmp(ICondition::sle),
            ops()->ICmp(ICondition::ule),
        };
        return EmitComparsion(node, RuntimeLib::StringLE, candidate, arraysize(candidate));
    }
    
    int VisitGreater(cpl::Greater *node) override {
        Operator *candidate[3] = {
            ops()->FCmp(FCondition::ugt),
            ops()->ICmp(ICondition::sgt),
            ops()->ICmp(ICondition::ugt),
        };
        return EmitComparsion(node, RuntimeLib::StringGT, candidate, arraysize(candidate));
    }
    
    int VisitGreaterEqual(cpl::GreaterEqual *node) override {
        Operator *candidate[3] = {
            ops()->FCmp(FCondition::uge),
            ops()->ICmp(ICondition::sge),
            ops()->ICmp(ICondition::uge),
        };
        return EmitComparsion(node, RuntimeLib::StringGE, candidate, arraysize(candidate));
    }
    
    int VisitIntLiteral(cpl::IntLiteral *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::Int32, ops()->I32Constant(node->value()));
        return Returning(val);
    }
    
    int VisitUIntLiteral(cpl::UIntLiteral *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::UInt32, ops()->U32Constant(node->value()));
        return Returning(val);
    }
    
    int VisitCharLiteral(cpl::CharLiteral *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::UInt32, ops()->U32Constant(node->value()));
        return Returning(val);
    }
    
    int VisitBoolLiteral(cpl::BoolLiteral *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::Int8, ops()->I8Constant(node->value()));
        return Returning(val);
    }
    
    int VisitUnitLiteral(cpl::UnitLiteral *node) override { return Returning(Unit()); }
    
    int VisitEmptyLiteral(cpl::EmptyLiteral *node) override { return Returning(Nil()); }
    
    int VisitI64Literal(cpl::I64Literal *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::Int64, ops()->I64Constant(node->value()));
        return Returning(val);
    }
    
    int VisitU64Literal(cpl::U64Literal *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::UInt64, ops()->U64Constant(node->value()));
        return Returning(val);
    }
    
    int VisitF32Literal(cpl::F32Literal *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::Float32, ops()->F32Constant(node->value()));
        return Returning(val);
    }
    
    int VisitF64Literal(cpl::F64Literal *node) override {
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::Float64, ops()->F64Constant(node->value()));
        return Returning(val);
    }
    
    int VisitStringLiteral(cpl::StringLiteral *node) override {
        auto kstr = node->value()->Duplicate(arena());
        auto val = Value::New0(arena(), SourcePosition::Unknown(), Types::String, ops()->StringConstant(kstr));
        return Returning(val);
    }
    
    int VisitOptionLiteral(cpl::OptionLiteral *node) override {
        //node->type()
        auto type = BuildType(node->type());
        if (node->is_some()) {
            Value *value = nullptr;
            if (ReduceReturningOnlyOne(node->value(), &value) < 0) {
                return -1;
            }
            return Returning(value);
        } else {
            return Returning(Nil(type));
        }
    }
    
    static Function::Decoration ToDecoration(const cpl::FunctionDeclaration *ast) {
        if (!ast->body()) {
            return Function::kAbstract;
        }
        switch (ast->decoration()) {
            case cpl::FunctionDeclaration::kDefault:
                return Function::kDefault;
            case cpl::FunctionDeclaration::kAbstract:
                return Function::kAbstract;
            case cpl::FunctionDeclaration::kNative:
                return Function::kNative;
            case cpl::FunctionDeclaration::kOverride:
                return Function::kOverride;
            default:
                UNREACHABLE();
                break;
        }
    }
private:
    bool ok() { return status_.ok(); }
    bool fail() { return status_.fail(); }
    base::Arena *arena() { return owns_->arena_; }
    cpl::SyntaxFeedback *feedback() { return owns_->error_feedback_; }
    OperatorsFactory *ops() { return owns_->ops_; }
    
    LoopContext *loop() { return DCHECK_NOTNULL(loop_or_null()); }
    LoopContext *loop_or_null() { return emitting_->current_loop(); }

    BasicBlock *b() const { return DCHECK_NOTNULL(DCHECK_NOTNULL(emitting_)->current_block()); }
    void b(BasicBlock *blk) { DCHECK_NOTNULL(emitting_)->set_current_block(blk); }
    
    void SetAndMakeLast(BasicBlock *blk) {
        DCHECK_NOTNULL(emitting_)->fun()->MoveToLast(blk);
        b(blk);
    }
    
    std::string MakeFullName(const String *name) const {
        return module_->full_name()->ToString().append(".").append(name->ToString());
    }
    
    void HandleMergeConflictsV2(std::string_view name, std::vector<Conflict> &&paths, BasicBlock *blk,
                                std::map<Value *, Value *> *updates) {
        auto origin = location_->FindSymbol(name);
        assert(origin.IsFound());
        std::vector<Node *> inputs;
        for (auto path : paths) {
            inputs.push_back(path.value);
        }
        for (auto path : paths) {
            inputs.push_back(/*!path.path ? b() : path.path*/path.path);
        }
        auto op = ops()->Phi(static_cast<int>(paths.size()), static_cast<int>(paths.size()));
        auto phi = blk->NewNodeWithNodes(nullptr, SourcePosition::Unknown(), origin.core.value->type(), op, inputs);
        blk->MoveToFront(phi);
        location_->PutSymbol(name, Symbol::Val(origin.owns, phi, b()));
        (*updates)[origin.core.value] = phi;
    }
    
    void HandleMergeConflicts(std::string_view name, std::vector<Conflict> &&paths) {
        auto origin = location_->FindSymbol(name);
        assert(origin.IsFound());
        std::vector<Node *> inputs;
        for (auto path : paths) {
            inputs.push_back(path.value);
        }
        for (auto path : paths) {
            inputs.push_back(/*!path.path ? b() : path.path*/path.path);
        }
        auto op = ops()->Phi(static_cast<int>(paths.size()), static_cast<int>(paths.size()));
        auto phi = b()->NewNodeWithNodes(nullptr, SourcePosition::Unknown(), origin.core.value->type(), op, inputs);
        location_->PutSymbol(name, Symbol::Val(origin.owns, phi, b()));
    }
    
    int ReduceValuesOfBranches(const std::vector<Type> &types,
                               BasicBlock *origin,
                               BasicBlock *branchs_rows[],
                               std::vector<Value *> values_rows[],
                               int number_of_branchs,
                               int number_of_branchs_without_else,
                               std::vector<Value *> *results,
                               SourcePosition source_position) {
        if (types.size() == 1 && types[0].kind() == Type::kVoid) {
            return Returning(Unit());
        }
        
        const size_t max_cols = types.size();
        std::vector<std::vector<Value *>> branchs_cols(max_cols);
        for (size_t i = 0; i < max_cols; i++) {
            auto col = &branchs_cols[i];
            for (size_t j = 0; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                if (j < number_of_branchs && i < values_rows[j].size()) {
                    col->push_back(values_rows[j][i]);
                } else {
                    col->push_back(Unit());
                }
            }
        }
        
        std::vector<std::vector<Node *>> nodes(max_cols);
        for (size_t i = 0; i < max_cols; i++) {
            bool all_unit = true;
            bool at_least_one_unit = false;
            
            for (size_t j = 0; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                if (branchs_cols[i][j]->type().kind() == Type::kVoid) {
                    at_least_one_unit = true;
                } else {
                    all_unit = false;
                }
            }
            
            if (all_unit) {
                continue;
            }
            
            if (at_least_one_unit) {
                assert(!all_unit);
                for (size_t j = 0; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                    Value *reduced = nullptr;
                    if (branchs_cols[i][j]->type().kind() != Type::kVoid) {
                        reduced = EmitCastingIfNeeded(types[i], branchs_cols[i][j], source_position);
                    } else {
                        reduced = Nil(types[i]);
                    }
                    
                    nodes[i].push_back(reduced);
                    nodes[i].push_back(branchs_rows[j]);
                }
            } else {
                if (branchs_cols[i].size() == 1) {
                    auto reduced = branchs_cols[i][0];
                    
                    nodes[i].push_back(Nil(types[i]));
                    nodes[i].push_back(origin);
                    nodes[i].push_back(reduced);
                    nodes[i].push_back(branchs_rows[0]);
                } else {
                    for (size_t j = 0; j < number_of_branchs_without_else + 1/*else branch*/; j++) {
                        auto reduced = EmitCastingIfNeeded(types[i], branchs_cols[i][j], source_position);
                        nodes[i].push_back(reduced);
                        nodes[i].push_back(branchs_rows[j]);
                    }
                }
            }
            
            std::vector<Node *> dummy(std::move(nodes[i]));
            for (size_t x = 0; x < dummy.size(); x += 2) {
                nodes[i].push_back(dummy[x]);
            }
            for (size_t x = 1; x < dummy.size(); x += 2) {
                nodes[i].push_back(dummy[x]);
            }
        }
        
        for (size_t i = 0; i < max_cols; i++) {
            if (types[i].kind() == Type::kVoid) {
                results->push_back(Unit());
                continue;
            }
            
            auto input = static_cast<int>(nodes[i].size() / 2);
            auto op = ops()->Phi(input, input);
            auto phi = b()->NewNodeWithNodes(nullptr, source_position, types[i], op, nodes[i]);
            results->push_back(phi);
        }
        return static_cast<int>(results->size());
    }
    
    int ProcessDotChainAssignment(cpl::Dot *ast, Value *rval) {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        // a.b.c.d
        // ["a","b","c","d"]
        // a.b().c.d = 1
        // %0 = load a
        // %1 = call_handle %0 <b>
        // %2 = load %1 <c>
        // %3 = store %2, 1 <d>
        //
        std::vector<const String *> parts;
        cpl::Statement *node = ast;
        while (node->IsDot()) {
            auto dot = node->AsDot();
            parts.insert(parts.begin(), dot->field());
            node = dot->primary();
        }
        // `node` is first part of dot chain
        Value *primary = nullptr;
        if (ReduceReturningOnlyOne(node, &primary) < 0) {
            return -1;
        }
        assert(primary->type().kind() == Type::kValue || primary->type().IsReference());
        
        // a.b.c.d = x
        // ["b", "c", "d"]
        // %a.0 = load a
        // %b.1 = load %a.0 <b>
        // %c.2 = load %b.1 <c>
        // %c.3 = store %c.2, %x <d>
        // %b.4 = store %b.1, %c.3 <c>
        // %a.5 = store %a.0, %b.4 <b>
        // store a, %a.5
        // ----------------
        // bar.baz.i = x
        // ["baz", "i"]
        // %bar.0 = load bar
        // %baz.1 = load %bar.0 <baz>
        // %baz.2 = store %baz.1, %x <i>
        // %bar.3 = store %bar.0, %baz.2 <baz>
        // store bar, %bar.3
        Value *value = primary;
        std::stack<Value *> records;
        //records.push(value);
        for (size_t i = 0; i < parts.size() - 1; i++) {
            auto handle = DCHECK_NOTNULL(value->type().model()->FindMemberOrNull(parts[i]->ToSlice()));
            records.push(value);
            value = EmitLoadField(value, handle, root_ss.Position());
            assert(value->type().kind() == Type::kValue || value->type().kind() == Type::kReference);
        } {
            auto handle = DCHECK_NOTNULL(value->type().model()->FindMemberOrNull(parts.back()->ToSlice()));
            value = EmitStoreField(value, rval, handle, root_ss.Position());
        }
        for (int64_t i = static_cast<int64_t>(parts.size()) - 2; i >= 0; i--) {
            auto lval = records.top();
            records.pop();
            auto handle = DCHECK_NOTNULL(lval->type().model()->FindMemberOrNull(parts[i]->ToSlice()));
            value = EmitStoreField(lval, value, handle, root_ss.Position());
        }
        
        if (auto id = node->AsIdentifier()) {
            SourcePositionTable::Scope ss(id->source_position(), &root_ss);
            auto symbol = location_->FindSymbol(id->name()->ToSlice());
            assert(symbol.IsFound());
            if (symbol.owns->IsFileUnitScope()) { // is global?
                b()->NewNode(ss.Position(), Types::Void, ops()->StoreGlobal(), value);
            } else {
                location_->PutSymbol(id->name()->ToSlice(), Symbol::Val(symbol.owns, value, b()));
            }
        }
        return 0;
    }
    
    Value *EmitCastingIfNeeded(Type dest, Value *src, SourcePosition source_position) {
        if (dest.kind() == src->type().kind() && dest.model() == src->type().model()) {
            return src;
        }
        if (dest.IsPointer() && src->type().IsNotPointer()) {
            src = b()->NewNode(source_position, dest, ops()->LoadAddress(), src);
        } else if (dest.IsNotPointer() && src->type().IsPointer()) {
            src = b()->NewNode(source_position, dest, ops()->Deref(), src);
        }
        
        Operator *op = nullptr;
        auto const hint = GetConversionHint(dest, src->type());
        switch (hint) {
#define DEFINE_CASE(name) case k##name: op = ops()->name(); break;
                DECLARE_IR_CONVERSION(DEFINE_CASE)
#undef DEFINE_CASE
            case kKeep:
                return src;
                
            case kDeny:
            default:
                UNREACHABLE();
                break;
        }
        return b()->NewNode(source_position, dest, op, src);
    }
    
    void EmitUnreachable(SourcePosition souce_position = SourcePosition::Unknown()) {
        auto op = ops()->Unreachable();
        b()->NewNode(souce_position, Types::Void, op);
    }
    
    void EmitCallingModuleDependentInitializers(cpl::Package *node) {
        for (auto [name, import] : node->imports()) {
            std::string full_name(name);
            full_name.append(":").append(import.pkg->name()->ToString());
            
            auto deps = owns_->AssertedGetModule(full_name);
            auto init = DCHECK_NOTNULL(deps->FindFunOrNull(cpl::kModuleInitFunName));
            auto op = ops()->Closure(init);
            auto type = Type::Ref(init->prototype());
            auto fun = init_blk_->NewNode(SourcePosition::Unknown(), type, op);

            op = ops()->StringConstant(deps->full_name());
            auto pkg_name = Value::New0(arena(), SourcePosition::Unknown(), Types::String, op);
            
            op = ops()->CallRuntime(0, 2, RuntimeLib::PkgInitOnce);
            init_blk_->NewNode(SourcePosition::Unknown(), Types::Void, op, fun, pkg_name);
        }
    }
    
    Value *EmitCall(Operator *op, PrototypeModel *proto, std::vector<Value *> &&args,
                    std::vector<Value *> *returning_vals, SourcePosition source_position) {
        if (proto->vargs()) {
            // TODO:
            UNREACHABLE();
        }
        assert(args.size() == proto->params_size());
        for (size_t i = 0; i < proto->params_size(); i++) {
            args[i] = EmitCastingIfNeeded(proto->param(i), args[i], source_position);
        }
        
        auto type = proto->return_type(0);
        auto call = b()->NewNodeWithValues(nullptr, source_position, type, op, args);
        returning_vals->push_back(call);
        for (int i = 1; i < proto->return_types_size(); i++) {
            op = ops()->ReturningVal(i);
            auto rv = b()->NewNode(source_position, proto->return_type(i), op, call);
            returning_vals->push_back(rv);
        }
        return call;
    }
    
    Value *EmitLoadField(Value *value, Handle *handle, SourcePosition source_position) {
        Operator *op = nullptr;
        if (value->type().IsReference()) {
            op = ops()->LoadEffectField(handle);
        } else if (value->type().IsPointer()) {
            assert(value->type().kind() == Type::kValue);
            op = ops()->LoadAccessField(handle);
        } else {
            assert(value->type().kind() == Type::kValue);
            op = ops()->LoadInlineField(handle);
        }
        auto field = std::get<const Model::Field *>(handle->owns()->GetMember(handle));
        return b()->NewNode(source_position, field->type, op, value);
    }
    
    Value *EmitStoreField(Value *value, Value *input, Handle *handle, SourcePosition source_position) {
        Operator *op = nullptr;
        if (value->type().IsReference()) {
            op = ops()->StoreEffectField(handle);
        } else if (value->type().IsPointer()) {
            assert(value->type().kind() == Type::kValue);
            op = ops()->StoreAccessField(handle);
        } else {
            assert(value->type().kind() == Type::kValue);
            op = ops()->StoreInlineField(handle);
        }
        auto type = handle->owns()->constraint() == Model::kRef
        ? Type::Ref(const_cast<Model *>(handle->owns()))
        : Type::Val(const_cast<Model *>(handle->owns()));
        return b()->NewNode(source_position, type, op, value, input);
    }
    
    Function *GenerateFun(const cpl::FunctionDeclaration *ast, Model *owns, Function *fun = nullptr) {
        if (!fun) {
            auto full_name = String::New(arena(), ast->FullName());
            auto proto = owns_->BuildPrototype(ast->prototype(), down_cast<StructureModel>(owns));
            if (owns) {
                fun = module_->NewFunction(ToDecoration(ast), ast->name()->Duplicate(arena()),
                                           down_cast<StructureModel>(owns), proto);
            } else {
                fun = module_->NewFunction(ToDecoration(ast), ast->name()->Duplicate(arena()), full_name, proto);
            }
        } else {
            BuildType(ast->prototype()); // Build prototype always
        }
        
        auto entry = fun->NewBlock(String::New(arena(), "entry"));
        int hint = 0;
        
        FunContext emitter(&emitting_, fun, entry);
        FunctionScope scope(&location_, ast, fun);
        if (owns) {
            auto type = owns->constraint() == Model::kVal ? Type::Val(owns, true/*is_pointer*/) : Type::Ref(owns);
            auto op = ops()->Argument(hint++);
            auto arg = Value::New0(arena(), String::New(arena(), cpl::kThisName), SourcePosition::Unknown(), type, op);
            location_->PutValue(arg->name()->ToSlice(), arg);
            fun->mutable_paramaters()->push_back(arg);
        }
        for (auto param : ast->prototype()->params()) {
            assert(!cpl::Type::Is(param));
            auto item = static_cast<cpl::VariableDeclaration::Item *>(param);
            
            auto type = BuildType(item->type());
            auto op = ops()->Argument(hint++);
            auto arg = Value::New(arena(), SourcePosition::Unknown(), type, op);
            if (cpl::Identifier::IsPlaceholder(item->identifier())) {
                //continue;
            } else {
                arg->set_name(item->identifier()->Duplicate(arena()));
            }
            location_->PutValue(arg->name()->ToSlice(), arg);
            fun->mutable_paramaters()->push_back(arg);
        }
        if (ast->prototype()->vargs()) {
            auto type = Type::Ref(AssertedGetUdt<ArrayModel>(cpl::kAnyArrayFullName));
            auto op = ops()->Argument(hint++);
            auto arg = Value::New0(arena(), String::New(arena(), cpl::kArgsName), SourcePosition::Unknown(), type, op);
            location_->PutValue(arg->name()->ToSlice(), arg);
            fun->mutable_paramaters()->push_back(arg);
        }
        if (!ast->body()) {
            return fun;
        }
        
        std::vector<Value *> values;
        if (auto rs = Reduce(ast->body(), &values); rs < 0) {
            return nullptr;
        }
        
        if (ast->is_reduce()) {
            SourcePositionTable::Scope ss(CURRENT_SOUCE_POSITION(ast->body()));
            auto last_block = fun->blocks().back();
            auto op = ops()->Ret(static_cast<int>(values.size()));
            last_block->NewNodeWithValues(nullptr/*name*/, ss.Position(), Types::Void, op, values);
        }
        
        if (fun->prototype()->return_types_size() == 1 &&
            fun->prototype()->return_type(0).kind() == Type::kVoid) {
            SourcePositionTable::Scope ss(CURRENT_SOUCE_POSITION(ast->body()));
            b()->NewNode(ss.Position(), Types::Void, ops()->Ret(0));
        }
        fun->UpdateIdsOfBlocks();
        return fun;
    }
    
    void EmitLessOrLessEquals(Value *lhs, Value *rhs, bool is_close, BasicBlock *if_true, BasicBlock *if_false,
                              cpl::Node *ast) {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        assert(lhs->type().Equals(rhs->type()));
        Operator *op = nullptr;
        if (lhs->type().IsFloating()) {
            op = ops()->FCmp(is_close ? FCondition::ule : FCondition::ult);
        } else if (lhs->type().IsSigned()) {
            op = ops()->ICmp(is_close ? ICondition::sle : ICondition::slt);
        } else if (lhs->type().IsUnsigned()) {
            op = ops()->ICmp(is_close ? ICondition::ule : ICondition::ult);
        } else {
            UNREACHABLE();
        }
        auto cond = b()->NewNode(root_ss.Position(), Types::UInt8, op, lhs, rhs);
        b()->NewNode(root_ss.Position(), Types::Void, ops()->Br(1/*value_in*/, 2/*control_out*/), cond,
                     if_true, if_false);
        
    }
    
    void EmitGreaterOrGreaterEquals(Value *lhs, Value *rhs, bool is_close, BasicBlock *if_true, BasicBlock *if_false,
                                    cpl::Node *ast) {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        assert(lhs->type().Equals(rhs->type()));
        Operator *op = nullptr;
        if (lhs->type().IsFloating()) {
            op = ops()->FCmp(is_close ? FCondition::uge : FCondition::ugt);
        } else if (lhs->type().IsSigned()) {
            op = ops()->ICmp(is_close ? ICondition::sge : ICondition::sgt);
        } else if (lhs->type().IsUnsigned()) {
            op = ops()->ICmp(is_close ? ICondition::uge : ICondition::ugt);
        } else {
            UNREACHABLE();
        }
        auto cond = b()->NewNode(root_ss.Position(), Types::UInt8, op, lhs, rhs);
        b()->NewNode(root_ss.Position(), Types::Void, ops()->Br(1/*value_in*/, 2/*control_out*/), cond,
                     if_true, if_false);
        
    }
    
    void EmitEquals(Value *lhs, Value *rhs, BasicBlock *if_true, BasicBlock *if_false,
                    cpl::AstNode *ast) {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(ast));
        assert(lhs->type().Equals(rhs->type()));
        
        Value *cond = nullptr;
        if (lhs->type().IsFloating()) {
            auto op = ops()->FCmp(FCondition::ueq);
            cond = b()->NewNode(root_ss.Position(), Types::UInt8, op, lhs, rhs);
        } else if (lhs->type().IsIntegral()) {
            auto op = ops()->ICmp(ICondition::eq);
            cond = b()->NewNode(root_ss.Position(), Types::UInt8, op, lhs, rhs);
        } else if (lhs->type().kind() == Type::kString) {
            auto op = ops()->CallRuntime(1/*value_out*/, 2/*value_in*/, RuntimeLib::StringEQ);
            cond = b()->NewNode(root_ss.Position(), Types::UInt8, op, lhs, rhs);
        } else {
            UNREACHABLE();
        }
        b()->NewNode(root_ss.Position(), Types::Void, ops()->Br(1/*value_in*/, 2/*control_out*/), cond, if_true,
                     if_false);
    }
    
    int EmitComparsion(cpl::BinaryExpression *ast, RuntimeId rid, Operator *candidate[3], size_t n) {
        Value *lhs = nullptr, *rhs = nullptr;
        if (ReduceReturningOnlyOne(ast->lhs(), &lhs) < 0 || ReduceReturningOnlyOne(ast->rhs(), &rhs) < 0) {
            return -1;
        }
        assert(lhs->type().kind() == rhs->type().kind());
        
        SourcePositionTable::Scope ss(CURRENT_SOUCE_POSITION(ast));
        if (lhs->type().kind() == Type::kString) {
            auto op = ops()->CallRuntime(1/*value_out*/, 2/*value_in*/, rid);
            auto call = b()->NewNode(ss.Position(), Types::UInt8, op, lhs, rhs);
            return Returning(call);
        }
        Operator *op = nullptr;
        if (lhs->type().IsFloating()) {
            op = candidate[0];
        } else if (lhs->type().IsSigned()) {
            op = candidate[1];
        } else {
            op = n == 3 ? candidate[2] : candidate[1];
        }
        
        return Returning(b()->NewNode(ss.Position(), Types::UInt8, op, lhs, rhs));
    }
    
    bool ShouldCaptureVal(NamespaceScope *scope, Value *val) {
        if (scope->IsFileUnitScope() || scope->IsStructureScope()) {
            return false;
        }
        auto fun_scope = location_->NearlyFunctionScope();
        if (!fun_scope) {
            return false;
        }
        for (auto sp = fun_scope->prev(); sp != nullptr; sp = sp->prev()) {
            if (sp->IsFileUnitScope() || sp->IsStructureScope()) {
                break;
            }
            if (sp == scope) {
                return true;
            }
        }
        return false;
    }
    
    Type BuildType(const cpl::Type *ast) { return owns_->BuildType(ast); }
    
    int ReduceReturningAtLeastOne(cpl::AstNode *node, Value **receiver) {
        const int nrets = node->Accept(this);
        if (fail() || nrets < 0) {
            return -1;
        }
        *receiver = results_.top();
        int i = nrets - 1;
        while (i--) { results_.pop(); }
        return nrets;
    }
    
    int ReduceReturningOnlyOne(cpl::AstNode *node, Value **receiver) {
        const int nrets = node->Accept(this);
        if (fail() || nrets < 0) {
            return -1;
        }
        assert(nrets == 1);
        *receiver = results_.top();
        int i = nrets - 1;
        while (i--) { results_.pop(); }
        return nrets;
    }
    
    int Reduce(cpl::AstNode *node, std::vector<Value *> *receiver = nullptr) {
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
    
    int Returning(Value *val) {
        results_.push(DCHECK_NOTNULL(val));
        return 1;
    }
    
    int Returning(const std::vector<Value *> &vals) {
        for (auto val : vals) { results_.push(val); }
        return static_cast<int>(vals.size());
    }
    
    const String *CurrentFileName() const {
        return DCHECK_NOTNULL(location_->NearlyFileUnitScope())->file_unit()->file_name();
    }
    
    template<class T>
    inline T *AssertedGetUdt(std::string_view name) {
        return down_cast<T>(owns_->AssertedGetUdt(name));
    }
    
    Value *Nil() const { return DCHECK_NOTNULL(owns_->nil_val_); }
    
    Value *Nil(const Type &type) {
        auto op = ops()->NilConstant();
        assert(type.IsReference());
        return Value::New(arena(), SourcePosition::Unknown(), type, op);
    }
    
    Value *Unit() const { return DCHECK_NOTNULL(owns_->unit_val_); }
    
    Value *Bool(bool value) {
        return Value::New0(arena(), SourcePosition::Unknown(), Types::UInt8, ops()->U8Constant(value));
    }
    
    StructureModel *StringTy() const { return down_cast<StructureModel>(owns_->string_ty_); }
    
    IntermediateRepresentationGenerator *const owns_;
    NamespaceScope *location_ = nullptr;
    FunContext *emitting_ = nullptr;
    Module *module_ = nullptr;
    Function *init_fun_ = nullptr;
    BasicBlock *init_blk_ = nullptr;
    std::stack<Value *> results_;
    base::Status status_ = base::Status::OK();
}; // class IntermediateRepresentationGenerator::AstVisitor


IntermediateRepresentationGenerator::IntermediateRepresentationGenerator(const std::unordered_map<std::string_view, cpl::GlobalSymbol> &symbols,
                                                                         base::Arena *arena,
                                                                         OperatorsFactory *ops,
                                                                         cpl::Package *entry,
                                                                         cpl::SyntaxFeedback *error_feedback)
: ast_nodes_(symbols)
, arena_(DCHECK_NOTNULL(arena))
, entry_(entry)
, error_feedback_(error_feedback)
//, global_udts_(arena)
//, global_vars_(arena)
//, global_funs_(arena)
, symbols_(arena)
, modules_(arena)
, pkg_scopes_(arena)
, track_(arena)
, ops_(DCHECK_NOTNULL(ops)) {
}

base::Status IntermediateRepresentationGenerator::Run() {
    Prepare0();
    Prepare1();
    
    auto accept = [this] (cpl::Package *pkg) {
        IRGeneratorAstVisitor visitor(this);
        pkg->Accept(&visitor);
    };
    if (auto rs = RecursivePackage(entry_, std::move(accept)); rs.fail()) {
        return rs;
    }
    accept(entry_);
    return base::Status::OK();
}

base::Status IntermediateRepresentationGenerator::Prepare0() {
    using std::placeholders::_1;
    
    if (auto rs = RecursivePackage(entry_, std::bind(&IntermediateRepresentationGenerator::PreparePackage0, this, _1));
        rs.fail()) {
        return rs;
    }
    PreparePackage0(entry_);
    
    nil_val_ = Value::New0(arena_, SourcePosition::Unknown(), Type::Ref(AssertedGetUdt(cpl::kAnyClassFullName)),
                           ops_->NilConstant());
    unit_val_ = Value::New0(arena_, SourcePosition::Unknown(), Types::Void, ops_->NilConstant());
    true_val_ = Value::New0(arena_, SourcePosition::Unknown(), Types::UInt8, ops_->U8Constant(1));
    false_val_ = Value::New0(arena_, SourcePosition::Unknown(), Types::UInt8, ops_->U8Constant(0));
    string_ty_ = AssertedGetUdt(cpl::kStringClassFullName);
    return base::Status::OK();
}

base::Status IntermediateRepresentationGenerator::Prepare1() {
    using std::placeholders::_1;
    
    if (auto rs = RecursivePackage(entry_, std::bind(&IntermediateRepresentationGenerator::PreparePackage1, this, _1));
        rs.fail()) {
        return rs;
    }
    PreparePackage1(entry_);
    return base::Status::OK();
}

void IntermediateRepresentationGenerator::PreparePackage0(cpl::Package *pkg) {
    std::string full_name(pkg->path()->ToString());
    full_name.append(":").append(pkg->name()->ToString());
    
    //printd("%s", full_name.c_str());
    if (auto iter = modules_.find(full_name); iter != modules_.end()) {
        return; // Ignore duplicated
    }
    
    auto module = new (arena_) Module(arena_,
                                      pkg->name()->Duplicate(arena_),
                                      String::New(arena_, full_name),
                                      pkg->path()->Duplicate(arena_),
                                      pkg->full_path()->Duplicate(arena_));
    modules_[module->full_name()->ToSlice()] = module;
    
    
    for (auto file_unit : pkg->source_files()) {
        for (auto def : file_unit->class_defs()) {
            if (!def->generic_params().empty()) {
                continue;
            }
            std::string name(full_name + "." + def->name()->ToString());
            auto model = module->NewClassModel(def->name()->Duplicate(arena_), String::New(arena_, name),
                                               nullptr/*base_of*/);
            symbols_[model->full_name()->ToSlice()] = Symbol::Udt(nullptr, model, def);
        }
        
        for (auto def : file_unit->struct_defs()) {
            if (!def->generic_params().empty()) {
                continue;
            }
            std::string name(full_name + "." + def->name()->ToString());
            auto model = module->NewStructModel(def->name()->Duplicate(arena_), String::New(arena_, name),
                                                nullptr/*base_of*/);
            symbols_[model->full_name()->ToSlice()] = Symbol::Udt(nullptr, model, def);
        }
        
        for (auto def : file_unit->interfaces()) {
            if (!def->generic_params().empty()) {
                continue;
            }
            std::string name(full_name + "." + def->name()->ToString());
            auto model = module->NewInterfaceModel(def->name()->Duplicate(arena_), String::New(arena_, name));
            symbols_[model->full_name()->ToSlice()] = Symbol::Udt(nullptr, model, def);
        }
    }
}

void IntermediateRepresentationGenerator::PreparePackage1(cpl::Package *pkg) {
    std::string full_name(pkg->path()->ToString());
    full_name.append(":").append(pkg->name()->ToString());
    
    auto iter = modules_.find(full_name);
    assert(iter != modules_.end());
    auto module = iter->second;
    if (Track(module, 1/*dest*/)) {
        return;
    }
    auto init = InstallInitFun(module);
    
    if (auto iter = symbols_.find(cpl::kAnyArrayFullName); iter == symbols_.end()) {
        auto any = AssertedGetUdt(cpl::kAnyClassFullName);
        auto name = String::New(arena_, "Any[]");
        auto full_name = String::New(arena_, cpl::kAnyArrayFullName);
        auto any_array = new (arena_) ArrayModel(arena_, name, full_name, 1, Type::Ref(any));
        symbols_[any_array->full_name()->ToSlice()] = Symbol::Udt(nullptr, any_array);
    }
    
    for (auto file_unit : pkg->source_files()) {
        for (auto var : file_unit->vars()) {
            for (auto i = 0; i < var->ItemSize(); i++) {
                SourcePositionTable::Scope scope(file_unit->file_name()->ToSlice(), var->source_position(),
                                                 module->mutable_source_position_table());
                
                auto it = var->AtItem(i);
                auto name = String::New(arena_, full_name + "." + it->Identifier()->ToString());
                auto val = Value::New0(arena_, name, scope.Position(), BuildType(it->Type()), ops_->GlobalValue(name));
                module->InsertGlobalValue(it->Identifier()->Duplicate(arena_), val);
                symbols_[name->ToSlice()] = Symbol::Val(nullptr/*owns*/, val, nullptr/*block*/, var);
            }
        }
        
        for (auto var : file_unit->objects()) {
            SourcePositionTable::Scope scope(file_unit->file_name()->ToSlice(), var->source_position(),
                                             module->mutable_source_position_table());
            
            auto name = String::New(arena_, full_name + "." + var->Identifier()->ToString());
            auto op = ops_->LazyValue(name);
            auto val = Value::New0(arena_, name, scope.Position(), BuildType(var->Type()), op);
            module->InsertGlobalValue(var->Identifier()->Duplicate(arena_), val);
            symbols_[name->ToSlice()] = Symbol::Val(nullptr/*owns*/, val, nullptr/*block*/, var);
        }
        
        for (auto def : file_unit->funs()) {
            if (!def->generic_params().empty()) {
                continue;
            }
            auto ty = BuildType(def->prototype());
            auto fun = module->NewFunction(IRGeneratorAstVisitor::ToDecoration(def),
                                           def->name()->Duplicate(arena_),
                                           String::New(arena_, full_name + "." + def->name()->ToString()),
                                           down_cast<PrototypeModel>(ty.model()));
            symbols_[fun->full_name()->ToSlice()] = Symbol::Fun(nullptr, fun, def);
        }
    }
    
    pkg_scopes_[pkg] = new PackageScope(nullptr/*location*/, pkg, &symbols_);
}

Function *IntermediateRepresentationGenerator::InstallInitFun(Module *module) {
    auto name = String::New(arena_, cpl::kModuleInitFunName);
    std::string buf(module->full_name()->ToString().append(".").append(name->ToString()));
    auto full_name = String::New(arena_, buf);
    
    PrototypeModel *proto = nullptr;
    if (auto model = FindUdtOrNull(cpl::kModuleInitFunProtoName)) {
        proto = down_cast<PrototypeModel>(model);
    } else {
        proto = new (arena_) PrototypeModel(arena_, String::New(arena_, cpl::kModuleInitFunProtoName), false/*vargs*/);
        proto->mutable_return_types()->push_back(Types::Void);
        symbols_[proto->full_name()->ToSlice()] = Symbol::Udt(nullptr, proto);
    }
    
    auto init = module->NewFunction(Function::kDefault, name, full_name, proto);
    init->NewBlock(String::New(arena_, "boot"));
    symbols_[init->full_name()->ToSlice()] = Symbol::Fun(nullptr, init);
    return init;
}

Type IntermediateRepresentationGenerator::BuildType(const cpl::Type *type) {
    switch (type->primary_type()) {
        case cpl::Type::kType_i8:
            return Types::Int8;
        case cpl::Type::kType_u8:
            return Types::UInt8;
        case cpl::Type::kType_i16:
            return Types::Int16;
        case cpl::Type::kType_u16:
            return Types::UInt16;
        case cpl::Type::kType_i32:
            return Types::Int32;
        case cpl::Type::kType_u32:
            return Types::UInt32;
        case cpl::Type::kType_i64:
            return Types::Int64;
        case cpl::Type::kType_u64:
            return Types::UInt64;
        case cpl::Type::kType_f32:
            return Types::Float32;
        case cpl::Type::kType_f64:
            return Types::Float64;
        case cpl::Type::kType_char:
            return Types::UInt32;
        case cpl::Type::kType_bool:
            return Types::UInt8;
        case cpl::Type::kType_any:
            return Type::Ref(AssertedGetUdt(cpl::kAnyClassFullName));
        case cpl::Type::kType_class: {
            auto clazz = type->AsClassType();
            return Type::Ref(AssertedGetUdt(clazz->definition()->FullName()));
        } break;
        case cpl::Type::kType_struct: {
            auto clazz = type->AsStructType();
            return Type::Val(AssertedGetUdt(clazz->definition()->FullName()));
        } break;
        case cpl::Type::kType_string:
            return Type::Ref(string_ty_);
        case cpl::Type::kType_array: {
            auto ast_ty = type->AsArrayType();
            auto element_ty = BuildType(ast_ty->element_type());
            std::string full_name(ArrayModel::ToString(ast_ty->dimension_count(), element_ty));
            if (auto ar = FindUdtOrNull(full_name)) {
                return Type::Ref(ar);
            }
            auto name = String::New(arena_, full_name);
            auto ar = new (arena_) ArrayModel(arena_, name, name, ast_ty->dimension_count(),
                                              element_ty);
            symbols_[ar->full_name()->ToSlice()] = Symbol::Udt(nullptr, ar);
            return Type::Ref(ar);
        } break;
        case cpl::Type::kType_option: {
            auto ast_ty = type->AsOptionType();
            auto element_ty = BuildType(ast_ty->element_type());
            assert(element_ty.IsReference() || element_ty.kind() == Type::kValue);
            return Type::Ref(Boxing(element_ty), true/*nullable*/);
        } break;
        case cpl::Type::kType_channel:
            UNREACHABLE(); // TODO:
            break;
        case cpl::Type::kType_function:
            return Type::Ref(BuildPrototype(type->AsFunctionPrototype()));
        case cpl::Type::kType_interface: {
            auto ast_ty = type->AsInterfaceType();
            return Type::Val(AssertedGetUdt(ast_ty->definition()->FullName()));
        } break;
        case cpl::Type::kType_unit:
            return Types::Void;
        case cpl::Type::kType_none:
        case cpl::Type::kType_symbol:
        default:
            UNREACHABLE();
            break;
    }
}

Model *IntermediateRepresentationGenerator::Boxing(Type type) {
    switch (type.kind()) {
        case Type::kInt32:
            return AssertedGetUdt(cpl::kI32ClassFullName);
        case Type::kUInt32:
            return AssertedGetUdt(cpl::kU32ClassFullName);
        case Type::kString:
            return AssertedGetUdt(cpl::kStringClassFullName);
        case Type::kReference:
        case Type::kValue:
            return type.model();
        default:
            UNREACHABLE();
            break;
    }
    return nullptr;
}

PrototypeModel *IntermediateRepresentationGenerator::BuildPrototype(const cpl::FunctionPrototype *ast,
                                                                    StructureModel *owns) {
    std::vector<Type> params;
    if(owns) {
        if (owns->declaration() == Model::kClass) {
            params.push_back(Type::Ref(owns));
        } else {
            assert(owns->declaration() == Model::kStruct);
            params.push_back(Type::Val(owns, true/*is_pointer*/));
        }
    }
    
    for (auto ty : ast->params()) {
        if (cpl::Type::Is(ty)) {
            params.push_back(BuildType(static_cast<cpl::Type *>(ty)));
        } else {
            auto item = static_cast<cpl::VariableDeclaration::Item *>(ty);
            params.push_back(BuildType(item->type()));
        }
    }
    
    if (ast->vargs()) {
        params.push_back(Type::Ref(AssertedGetUdt(cpl::kAnyArrayFullName)));
    }
    
    std::vector<Type> return_types;
    for (auto ty : ast->return_types()) {
        return_types.push_back(BuildType(ty));
    }
    auto full_name(PrototypeModel::ToString(&params[0], params.size(), ast->vargs(),
                                            &return_types[0], return_types.size()));
    if (auto fun = FindUdtOrNull(full_name)) {
        return down_cast<PrototypeModel>(fun);
    }
    auto name = String::New(arena_, full_name);
    auto fun = new (arena_) PrototypeModel(arena_, name, ast->vargs());
    for (auto ty : params) { fun->mutable_params()->push_back(ty); }
    for (auto ty : return_types) { fun->mutable_return_types()->push_back(ty); }
    return fun;
}

base::Status IntermediateRepresentationGenerator::RecursivePackage(cpl::Package *root,
                                                                   std::function<void(cpl::Package *)> &&callback) {
    if (root->IsTerminator()) {
        return base::Status::OK();
    }
    
    for (auto pkg : root->dependences()) {
        if (auto rs = RecursivePackage(pkg, std::move(callback)); rs.fail()) {
            return rs;
        }
        callback(pkg);
    }
    return base::Status::OK();
}

} // namespace ir

} // namespace yalx
