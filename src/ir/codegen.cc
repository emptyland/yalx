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


class IRGeneratorAstVisitor : public cpl::AstVisitor {
public:
    IRGeneratorAstVisitor(IntermediateRepresentationGenerator *owns): owns_(owns) {}
    
    int VisitPackage(cpl::Package *node) override {
        assert(module_ == nullptr);
        
        auto name = node->path()->ToString().append(":").append(node->name()->ToString());
        module_ = DCHECK_NOTNULL(owns_->AssertedGetModule(name));
        init_fun_ = DCHECK_NOTNULL(module_->FindFunOrNull(cpl::kModuleInitFunName));
        init_blk_ = DCHECK_NOTNULL(init_fun_->entry());
        
        auto scope = owns_->AssertedGetPackageScope(node);
        scope->Enter(&location_);
        feedback()->set_package_name(name);
        
        EmitCallingModuleDependentInitializers(node);
        
        for (auto file_unit : node->source_files()) {
            feedback()->set_file_name(file_unit->file_name()->ToString());
            if (auto rs = file_unit->Accept(this); rs < 0 || fail()) {
                scope->Exit();
                return -1;
            }
        }
        scope->Exit();
        return 0;
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
    
    int VisitClassDefinition(cpl::ClassDefinition *node) override { return GenerateStructureModel(node); }
    
    int VisitStructDefinition(cpl::StructDefinition *node) override { return GenerateStructureModel(node); }
    
    template<class T> int GenerateStructureModel(T *node) {
        if (!node->generic_params().empty()) {
            return Returning(Unit());
        }
        if (node->IsClassDefinition()) {
            printd("class %s", node->FullName().c_str());
        } else {
            printd("struct %s", node->FullName().c_str());
        }
        
        auto clazz = AssertedGetUdt<StructureModel>(node->FullName());
        if (node->base_of()) {
            if (ProcessDependencySymbolIfNeeded(node->base_of()) < 0) {
                return -1;
            }
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
            //printd("find method: %s", ast->name()->data());
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
            //printd("constructor: %s(%s)", ctor->name()->data(), ctor->full_name()->data());
            Model::Method method {
                .fun = ctor,
                .access = kPublic,
                .in_vtab = 0,
                .in_itab = 0,
            };
            clazz->InsertMethod(method);
        }
        
        // TODO:
        // UNREACHABLE();
        return Returning(Unit());
    }
    
    int VisitObjectDeclaration(cpl::ObjectDeclaration *node) override {
        SourcePositionTable::Scope ss_root(CURRENT_SOUCE_POSITION(node));
        assert(location_->IsFileUnitScope());
        
        auto full_name = MakeFullName(node->name());
        auto value = owns_->AssertedGetVal(full_name);
        auto op = ops()->StoreGlobal();
        location_->current_block()->NewNode(ss_root.Position(), Types::Void, op, value, Nil(value->type()));
        return Returning(Unit());
    }
    
    int VisitVariableDeclaration(cpl::VariableDeclaration *node) override {
        SourcePositionTable::Scope ss_root(CURRENT_SOUCE_POSITION(node));
        const bool in_global_space = location_->IsFileUnitScope();
        
        std::vector<Value *> init_vals;
        for (auto ast : node->initilaizers()) {
            if (Reduce(ast, &init_vals) < 0) {
                return -1;
            }
        }

        assert(init_vals.size() == node->ItemSize());
        for (auto i = 0; i < node->ItemSize(); i++) {
            auto ast = node->AtItem(i);
            SourcePositionTable::Scope ss(ast->source_position(), &ss_root);
            if (cpl::Identifier::IsPlaceholder(ast->Identifier())) {
                continue;
            }
            if (in_global_space) {
                auto dest = location_->FindSymbol(ast->Identifier()->ToSlice());
                assert(dest.IsFound());
                auto op = ops()->StoreGlobal();
                location_->current_block()->NewNode(ss.Position(), Types::Void, op, dest.core.value, init_vals[i]);
            } else {
                location_->PutValue(ast->Identifier()->ToSlice(), init_vals[i], ast);
            }
        }
        
        return Returning(Unit());
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
            auto ret = location_->current_block()->NewNode(ss.Position(), Types::Void, ops()->Ret(0));
            return Returning(ret);
        }
        
        std::vector<Value *> values;
        for (auto val : node->returnning_vals()) {
            if (auto rs = Reduce(val, &values); rs < 0) {
                return -1;
            }
        }
        auto ret = location_->current_block()->NewNodeWithValues(nullptr/*name*/, ss.Position(), Types::Void,
                                                                 ops()->Ret(static_cast<int>(values.size())),
                                                                 values);
        return Returning(ret);
    }
    
    int VisitIdentifier(cpl::Identifier *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        
        //printd("find symbol: %s", node->name()->data());
        auto symbol = location_->FindSymbol(node->name()->ToSlice());
        assert(symbol.IsFound());
        switch (symbol.kind) {
            case Symbol::kValue:
                return Returning(symbol.core.value);
            case Symbol::kHandle: {
                auto handle = symbol.core.handle;
                auto this_val = location_->FindSymbol(cpl::kThisName);
                assert(this_val.IsFound());
                return Returning(EmitLoadField(this_val.core.value, handle, root_ss.Position()));
            } break;
            case Symbol::kFun: // TODO:
                UNREACHABLE();
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
    
    int VisitCalling(cpl::Calling *node) override {
        SourcePositionTable::Scope ss(CURRENT_SOUCE_POSITION(node));
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
            switch (method->fun->decoration()) {
                case Function::kAbstract:
                    op = ops()->CallAbstract(handle, 1, value_in);
                    break;
                case Function::kOverride:
                    op = ops()->CallVirtual(handle, 1, value_in);
                    break;
                default:
                    op = ops()->CallHandle(handle, 1, value_in);
                    break;
            }
            std::vector<Value *> results;
            EmitCall(op, proto, std::move(args), &results, ss.Position());
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
            EmitCall(op, proto, std::move(args), &results, ss.Position());
            return Returning(results);
        }
        
        if (symbol.kind == Symbol::kFun) {
            auto proto = symbol.core.fun->prototype();
            auto value_in = static_cast<int>(proto->params_size());
            auto op = ops()->CallDirectly(symbol.core.fun, 1/*value_out*/, value_in);
            std::vector<Value *> results;
            EmitCall(op, proto, std::move(args), &results, ss.Position());
            return Returning(results);
        }
        
        if (symbol.kind == Symbol::kModel) {
            assert(symbol.core.model->declaration() == Model::kClass ||
                   symbol.core.model->declaration() == Model::kStruct);
            if (symbol.node && ProcessDependencySymbolIfNeeded(symbol.node) < 0) {
                return -1;
            }
            
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
            auto ob = location_->current_block()->NewNode(ss.Position(), type, op);
            
            auto proto = clazz->constructor()->prototype();
            auto value_in = static_cast<int>(proto->params_size());
            auto handle = DCHECK_NOTNULL(clazz->FindMemberOrNull(clazz->constructor()->name()->ToSlice()));
            op = ops()->CallHandle(handle, 1, value_in);
            std::vector<Value *> results;
            args.insert(args.begin(), ob);
            EmitCall(op, proto, std::move(args), &results, ss.Position());
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
                        location_->current_block()->NewNode(ss.Position(), Types::Void, ops()->StoreGlobal(),
                                                            symbol.core.value, rval);
                    } else if (ShouldCaptureVal(symbol.owns, symbol.core.value)) {
                        // TODO: Capture Val
                        UNREACHABLE();
                    } else {
                        location_->PutSymbol(ast->name()->ToSlice(), Symbol::Val(symbol.owns, rval, symbol.node));
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
                    location_->current_block()->NewNode(ss.Position(), Types::Void, ops()->StoreGlobal(), rval);
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
        
        
        return Returning(Unit());
    }
    
    int VisitIfExpression(cpl::IfExpression *node) override {
        SourcePositionTable::Scope root_ss(CURRENT_SOUCE_POSITION(node));
        BranchScope trunk(&location_, location_->current_block(), node);
        if (node->initializer()) {
            REDUCE(node->initializer());
        }
        
        Value *condition = nullptr;
        if (ReduceReturningAtLeastOne(node->condition(), &condition) < 0) {
            return -1;
        }
        std::vector<Value *> values;
        
        auto fun_scope = location_->NearlyFunctionScope();
        auto fun = fun_scope->fun();
        auto then_block = fun->NewBlock(String::New(arena(), "then"));
        auto else_block = fun->NewBlock(String::New(arena(), node->else_clause() ? "else" : "out"));
        auto out_block = node->else_clause() ? fun->NewBlock(String::New(arena(), "out")) : else_block;
        
        if (node->then_clause()) {
            SourcePositionTable::Scope ss(node->then_clause()->source_position(), &root_ss);
            trunk.current_block()->NewNode(ss.Position(), Types::Void, ops()->Br(1/*value_in*/, 2/*control_out*/),
                                           condition, then_block, else_block);

            auto br = trunk.Branch(node->then_clause(), then_block);
            NamespaceScope::Keeper<BranchScope> holder(br);
            if (Reduce(node->then_clause(), &values) < 0) {
                return -1;
            }
            
            trunk.current_block()->NewNode(ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/),
                                           out_block);
        }
        
        if (node->else_clause()) {
            SourcePositionTable::Scope ss(node->then_clause()->source_position(), &root_ss);
            auto br = trunk.Branch(node->then_clause(), else_block);
            NamespaceScope::Keeper<BranchScope> holder(br);
            if (Reduce(node->else_clause(), &values) < 0) {
                return -1;
            }
            trunk.current_block()->NewNode(ss.Position(), Types::Void, ops()->Br(0/*value_in*/, 1/*control_out*/),
                                           out_block);
        }
        
        std::vector<Type> types;
        for (auto ast : node->reduced_types()) { types.push_back(BuildType(ast)); }
        if (types.size() == 1 && types[0].kind() == Type::kVoid) {
            return Returning(Unit());
        }
        
        // TODO:
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
    
    Value *EmitCastingIfNeeded(Type dest, Value *src, SourcePosition source_position) {
        if (dest.kind() == src->type().kind() && dest.model() == src->type().model()) {
            return src;
        }
        if (dest.IsPointer() && src->type().IsNotPointer()) {
            src = location_->current_block()->NewNode(source_position, dest, ops()->LoadAddress(), src);
        } else if (dest.IsNotPointer() && src->type().IsPointer()) {
            src = location_->current_block()->NewNode(source_position, dest, ops()->Deref(), src);
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
        return location_->current_block()->NewNode(source_position, dest, op, src);
    }
    
    int VisitAnnotationDefinition(cpl::AnnotationDefinition *node) override { UNREACHABLE(); }
    int VisitAnnotationDeclaration(cpl::AnnotationDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotation(cpl::Annotation *node) override { UNREACHABLE(); }
    int VisitBreak(cpl::Break *node) override { UNREACHABLE(); }
    int VisitContinue(cpl::Continue *node) override { UNREACHABLE(); }

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
    int VisitGreater(cpl::Greater *node) override { UNREACHABLE(); }
    int VisitTesting(cpl::Testing *node) override { UNREACHABLE(); }
    int VisitNegative(cpl::Negative *node) override { UNREACHABLE(); }
    int VisitNotEqual(cpl::NotEqual *node) override { UNREACHABLE(); }
    int VisitBitwiseOr(cpl::BitwiseOr *node) override { UNREACHABLE(); }
    int VisitLessEqual(cpl::LessEqual *node) override { UNREACHABLE(); }
    int VisitBitwiseAnd(cpl::BitwiseAnd *node) override { UNREACHABLE(); }
    int VisitBitwiseShl(cpl::BitwiseShl *node) override { UNREACHABLE(); }
    int VisitBitwiseShr(cpl::BitwiseShr *node) override { UNREACHABLE(); }
    int VisitBitwiseXor(cpl::BitwiseXor *node) override { UNREACHABLE(); }
    int VisitIndexedGet(cpl::IndexedGet *node) override { UNREACHABLE(); }
    int VisitGreaterEqual(cpl::GreaterEqual *node) override { UNREACHABLE(); }
    int VisitLambdaLiteral(cpl::LambdaLiteral *node) override { UNREACHABLE(); }
    int VisitWhenExpression(cpl::WhenExpression *node) override { UNREACHABLE(); }
    int VisitBitwiseNegative(cpl::BitwiseNegative *node) override { UNREACHABLE(); }
    int VisitArrayInitializer(cpl::ArrayInitializer *node) override { UNREACHABLE(); }
    int VisitTryCatchExpression(cpl::TryCatchExpression *node) override { UNREACHABLE(); }
    int VisitOptionLiteral(cpl::OptionLiteral *node) override { UNREACHABLE(); }
    int VisitAssertedGet(cpl::AssertedGet *node) override { UNREACHABLE(); }
    int VisitChannelInitializer(cpl::ChannelInitializer *node) override { UNREACHABLE(); }
    
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
    
    std::string MakeFullName(const String *name) const {
        return module_->full_name()->ToString().append(".").append(name->ToString());
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
        assert(primary->type().kind() == Type::kValue || primary->type().kind() == Type::kReference);
        
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
                location_->current_block()->NewNode(ss.Position(), Types::Void, ops()->StoreGlobal(), value);
            } else {
                location_->PutSymbol(id->name()->ToSlice(), Symbol::Val(symbol.owns, value));
            }
        }
        return 0;
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
        auto call = location_->current_block()->NewNodeWithValues(nullptr, source_position, type, op, args);
        returning_vals->push_back(call);
        for (int i = 1; i < proto->return_types_size(); i++) {
            op = ops()->ReturningVal(i);
            auto rv = location_->current_block()->NewNode(source_position, proto->return_type(i), op, call);
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
        return location_->current_block()->NewNode(source_position, field->type, op, value);
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
        return location_->current_block()->NewNode(source_position, type, op, value, input);
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
        FunctionScope scope(&location_, ast, fun);
        if (owns) {
            auto type = owns->declaration() == Model::kStruct ? Type::Val(owns, true/*is_pointer*/) : Type::Ref(owns);
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
        return fun;
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
    
    Type BuildType(const cpl::Type *ast) {
        auto rs = ProcessDependencyTypeIfNeeded(ast);
        assert(rs >= 0);
        return owns_->BuildType(ast);
    }
    
    int ProcessDependencyTypeIfNeeded(const cpl::Type *ast) {
        switch (ast->primary_type()) {
            case cpl::Type::kType_class:
                return ProcessDependencySymbolIfNeeded(ast->AsClassType()->definition());
            case cpl::Type::kType_struct:
                return ProcessDependencySymbolIfNeeded(ast->AsStructType()->definition());
            case cpl::Type::kType_option:
                return ProcessDependencyTypeIfNeeded(ast->AsOptionType()->element_type());
            case cpl::Type::kType_array:
                return ProcessDependencyTypeIfNeeded(ast->AsArrayType()->element_type());
            case cpl::Type::kType_channel:
                return ProcessDependencyTypeIfNeeded(ast->AsChannelType()->element_type());
            case cpl::Type::kType_function: {
                auto proto = ast->AsFunctionPrototype();
                for (auto param : proto->params()) {
                    if (cpl::Type::Is(param)) {
                        if (auto rs = ProcessDependencyTypeIfNeeded(static_cast<cpl::Type *>(param)); rs < 0) {
                            return -1;
                        }
                    } else {
                        auto item = static_cast<cpl::VariableDeclaration::Item *>(param);
                        if (auto rs = ProcessDependencyTypeIfNeeded(item->type()); rs < 0) {
                            return -1;
                        }
                    }
                }
                for (auto ty : proto->return_types()) {
                    if (auto rs = ProcessDependencyTypeIfNeeded(ty); rs < 0) {
                        return -1;
                    }
                }
            } break;
            case cpl::Type::kType_symbol:
                UNREACHABLE();
                break;
            default:
                break;
        }
        return 0;
    }
    
    int ProcessDependencySymbolIfNeeded(cpl::Statement *ast) {
        if (ast->IsTemplate()) {
            return 0; // Ignore template
        }
        auto [owns, sym] = ast->Owns(true/*force*/);
        auto pkg = ast->Pack(true/*force*/);
        if (auto pkg_scope = owns_->AssertedGetPackageScope(pkg); pkg_scope->HasNotTracked(ast)) {
            if (owns && owns->IsFileUnit()) {
                auto current_pkg_scope = location_->NearlyPackageScope();
                auto current_file_scope = location_->NearlyFileUnitScope();

                if (owns != current_file_scope->file_unit()) {
                    //printd("nested reduce: %s", down_cast<FileUnit>(owns)->file_name()->data());
                    PackageScope *external_scope = nullptr;
                    FileUnitScope *scope = nullptr;
                    if (pkg != current_pkg_scope->pkg()) {
                        external_scope = owns_->AssertedGetPackageScope(pkg);
                        external_scope->Enter(&location_);
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
                    if (pkg_scope->Track(sym)) {
                        return 0;
                    }
                    if (auto rs = Reduce(sym); rs < 0) {
                        return -1;
                    }
                }
            }
        }
        return 0;
    }
    
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
    
    IntermediateRepresentationGenerator *const owns_;
    NamespaceScope *location_ = nullptr;
    Module *module_ = nullptr;
    Function *init_fun_ = nullptr;
    BasicBlock *init_blk_ = nullptr;
    std::stack<Value *> results_;
    base::Status status_ = base::Status::OK();
}; // class IntermediateRepresentationGenerator::AstVisitor


IntermediateRepresentationGenerator::IntermediateRepresentationGenerator(const std::unordered_map<std::string_view, cpl::GlobalSymbol> &symbols,
                                                                         base::Arena *arena,
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
, ops_(arena->New<OperatorsFactory>(arena)) {
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

    nil_val_ = Value::New0(arena_,
                           SourcePosition::Unknown(),
                           Type::Ref(AssertedGetUdt(cpl::kAnyClassFullName)),
                           ops_->NilConstant());
    unit_val_ = Value::New0(arena_,
                            SourcePosition::Unknown(),
                            Types::Void,
                            ops_->NilConstant());
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
                symbols_[name->ToSlice()] = Symbol::Val(nullptr, val, it);
            }
        }
        
        for (auto var : file_unit->objects()) {
            SourcePositionTable::Scope scope(file_unit->file_name()->ToSlice(), var->source_position(),
                                             module->mutable_source_position_table());
            
            auto name = String::New(arena_, full_name + "." + var->Identifier()->ToString());
            auto op = ops_->LazyValue(name);
            auto val = Value::New0(arena_, name, scope.Position(), BuildType(var->Type()), op);
            module->InsertGlobalValue(var->Identifier()->Duplicate(arena_), val);
            symbols_[name->ToSlice()] = Symbol::Val(nullptr, val, var);
        }
        
        for (auto def : file_unit->funs()) {
            auto ty = BuildType(def->prototype());
            auto fun = module->NewFunction(IRGeneratorAstVisitor::ToDecoration(def),
                                           def->name()->Duplicate(arena_),
                                           String::New(arena_, full_name + "." + def->name()->ToString()),
                                           down_cast<PrototypeModel>(ty.model()));
            symbols_[fun->full_name()->ToSlice()] = Symbol::Fun(nullptr, fun, def);
        }
    }
    
    pkg_scopes_[pkg] = new PackageScope(nullptr/*location*/, init->entry(), pkg, &symbols_);
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
            return Types::String;
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
            return Type::Ref(element_ty.model(), true/*nullable*/);
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
