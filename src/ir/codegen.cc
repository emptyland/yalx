#include "ir/codegen.h"
#include "ir/scope.h"
#include "ir/operators-factory.h"
#include "ir/operator.h"
#include "ir/node.h"
#include "ir/metadata.h"
#include "compiler/ast.h"
#include "compiler/syntax-feedback.h"
#include "compiler/constants.h"
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
        
        //module_ = new (arena()) Module(arena(), node->name(), node->path(), node->full_path());
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

    int VisitFileUnit(cpl::FileUnit *node) override {
        // TODO:
        return 0;
    }
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
    int VisitCharLiteral(cpl::CharLiteral *node) override { UNREACHABLE(); }
    int VisitChannelInitializer(cpl::ChannelInitializer *node) override { UNREACHABLE(); }
    
private:
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
    , global_udts_(arena)
    , global_vars_(arena)
    , global_funs_(arena)
    , modules_(arena)
    , track_(arena) {
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
    
    printd("%s", full_name.c_str());
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
            auto model = new (arena_) StructureModel(arena_,
                                                     def->name()->Duplicate(arena_),
                                                     String::New(arena_, name),
                                                     StructureModel::kClass,
                                                     module,
                                                     nullptr);
            global_udts_[model->full_name()->ToSlice()] = model;
        }
        
        for (auto def : file_unit->struct_defs()) {
            if (!def->generic_params().empty()) {
                continue;
            }
            std::string name(full_name + "." + def->name()->ToString());
            auto model = new (arena_) StructureModel(arena_,
                                                     def->name()->Duplicate(arena_),
                                                     String::New(arena_, name),
                                                     StructureModel::kStruct,
                                                     module,
                                                     nullptr);
            global_udts_[model->full_name()->ToSlice()] = model;
        }
        
        for (auto def : file_unit->interfaces()) {
            if (!def->generic_params().empty()) {
                continue;
            }
            std::string name(full_name + "." + def->name()->ToString());
            auto model = new (arena_) InterfaceModel(arena_,
                                                     def->name()->Duplicate(arena_),
                                                     String::New(arena_, name));
            global_udts_[model->full_name()->ToSlice()] = model;
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
    for (auto file_unit : pkg->source_files()) {
        for (auto def : file_unit->funs()) {
            std::string name(full_name + "." + def->name()->ToString());
            auto ty = BuildType(def->prototype());
            auto fun = module->NewFunction(def->name()->Duplicate(arena_),
                                           String::New(arena_, name),
                                           down_cast<PrototypeModel>(ty.model()));
            global_funs_[fun->full_name()->ToSlice()] = fun;
        }
    }
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
            global_udts_[ar->full_name()->ToSlice()] = ar;
            return Type::Ref(ar);
        } break;
        case cpl::Type::kType_option: {
            auto ast_ty = type->AsOptionType();
            auto element_ty = BuildType(ast_ty->element_type());
            assert(element_ty.is_reference() || element_ty.kind() == Type::kValue);
            return Type::Ref(element_ty.model(), true/*nullable*/);
        } break;
        case cpl::Type::kType_channel:
            UNREACHABLE(); // TODO:
            break;
        case cpl::Type::kType_function: {
            auto ast_ty = type->AsFunctionPrototype();
            std::vector<Type> params;
            for (auto ty : ast_ty->params()) {
                if (cpl::Type::Is(ty)) {
                    params.push_back(BuildType(static_cast<cpl::Type *>(ty)));
                } else {
                    auto item = static_cast<cpl::VariableDeclaration::Item *>(ty);
                    params.push_back(BuildType(item->type()));
                }
                
            }
            std::vector<Type> return_types;
            for (auto ty : ast_ty->return_types()) {
                return_types.push_back(BuildType(ty));
            }
            auto full_name(PrototypeModel::ToString(&params[0], params.size(), ast_ty->vargs(),
                                                    &return_types[0], return_types.size()));
            if (auto fun = FindUdtOrNull(full_name)) {
                return Type::Ref(fun);
            }
            auto name = String::New(arena_, full_name);
            auto fun = new (arena_) PrototypeModel(arena_, name, ast_ty->vargs());
            for (auto ty : params) { fun->mutable_params()->push_back(ty); }
            for (auto ty : return_types) { fun->mutable_return_types()->push_back(ty); }
            global_udts_[fun->full_name()->ToSlice()] = fun;
            return Type::Ref(fun);
        } break;
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
