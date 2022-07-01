#include "compiler/generics-instantiating.h"
#include "compiler/syntax-feedback.h"
#include "compiler/scope.h"
#include "compiler/ast.h"
#include <stack>

namespace yalx {

namespace cpl {

#define Feedback() FeedbackWith(__FILE__, __LINE__)

#define DECL_AND_INSTANTIATE(type, output, input) \
    type *output = nullptr; \
    if (auto __it = Instantiate((input)); !__it) { \
        return -1; \
    } else { \
        output = down_cast<type>(__it); \
    }(void)0

#define INSTANTIATE(output, input) \
    if (auto __it = Instantiate((input)); !__it) { \
        return -1; \
    } else { \
        output = down_cast<std::remove_pointer<decltype(output)>::type>(__it); \
    }(void)0

class GenericsInstantiatingVisitor : public AstVisitor {
public:
    GenericsInstantiatingVisitor(const String *actual_name,
                                 base::Arena *arena, SyntaxFeedback *feedback,
                                 GenericsInstantiating::Resolver *resolver,
                                 Statement *original,
                                 size_t argc, Type **argv)
        : arena_(DCHECK_NOTNULL(arena))
        , feedback_(feedback)
        , resolver_(resolver)
        , actual_name_(actual_name)
        , original_(original)
        , argc_(argc)
        , argv_(argv) {
    }
    
    DEF_PTR_GETTER(Statement, original);
    DEF_VAL_GETTER(base::Status, status);
    
    Statement *result() {
        assert(!results_.empty());
        return static_cast<Statement *>(results_.top());
    }

    int VisitInterfaceDefinition(InterfaceDefinition *node) override {
        assert(results_.empty());
        if (PrepareTemplate(node) < 0){
            return -1;
        }

        // InterfaceDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position);
        auto copied = new (arena_) InterfaceDefinition(arena_, MakeFullName(node->name()), node->source_position());
        copied->set_access(node->access());
        copied->set_owns(node->owns());
        copied->set_package(node->package());
        copied->set_annotations(node->annotations());
        copied->set_original(node);
        
        resolver_->FindOrInsert(copied->PackageName()->ToSlice(), copied->name()->ToSlice(), copied); // Insert first
        
        for (auto method : node->methods()) {
            DECL_AND_INSTANTIATE(FunctionDeclaration, it, method);
            copied->mutable_methods()->push_back(it);
        }

        return Return(copied);
    }
    
    
    int VisitStructDefinition(StructDefinition *node) override {
        assert(results_.empty());
        if (PrepareTemplate(node) < 0){
            return -1;
        }
        
        // StructDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
        auto copied = new (arena_) StructDefinition(arena_, MakeFullName(node->name()), node->source_position());
        copied->set_access(node->access());
        copied->set_owns(node->owns());
        copied->set_package(node->package());
        copied->set_annotations(node->annotations());
        copied->set_original(node);
        
        resolver_->FindOrInsert(copied->PackageName()->ToSlice(), copied->name()->ToSlice(), copied); // Insert first
        IncompletableDefinition *base_of = nullptr;
        if (ProcessIncompletableDefinition(node, copied, &base_of) < 0) {
            return -1;
        }
        if (base_of) {
            if (!base_of->IsStructDefinition()) {
                Feedback()->Printf(node->super_calling()->source_position(), "Only struct can be inherit of struct");
                return -1;
            }
            copied->set_base_of(DCHECK_NOTNULL(base_of->AsStructDefinition()));
        }
        
        return Return(copied);
    }
    
    int VisitClassDefinition(ClassDefinition *node) override {
        assert(results_.empty());
        if (PrepareTemplate(node) < 0){
            return -1;
        }
        
        // ClassDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position)
        auto copied = new (arena_) ClassDefinition(arena_, MakeFullName(node->name()), node->source_position());
        copied->set_access(node->access());
        copied->set_owns(node->owns());
        copied->set_package(node->package());
        copied->set_annotations(node->annotations());
        copied->set_original(node);
        
        resolver_->FindOrInsert(copied->PackageName()->ToSlice(), copied->name()->ToSlice(), copied); // Insert first
        IncompletableDefinition *base_of = nullptr;
        if (ProcessIncompletableDefinition(node, copied, &base_of) < 0) {
            return -1;
        }
        if (base_of) {
            if (!DCHECK_NOTNULL(base_of)->IsClassDefinition()) {
                Feedback()->Printf(node->super_calling()->source_position(), "Only class can be inherit of class");
                return -1;
            }
            copied->set_base_of(DCHECK_NOTNULL(base_of->AsClassDefinition()));
        }
        
        for (auto concept : node->concepts()) {
            if (auto type = TypeLink(concept); !type) {
                return -1;
            } else {
                if (!type->IsInterfaceType()) {
                    Feedback()->Printf(concept->source_position(), "Only interface can be implements of class");
                    return -1;
                }
                copied->mutable_concepts()->push_back(type);
            }
        }

        // class Ref<T> (val T body)
        return Return(copied);
    }

    
    int VisitFunctionDeclaration(FunctionDeclaration *node) override {
        if (node == original() && PrepareTemplate(node) < 0){
            return -1;
        }

        // FunctionPrototype(base::Arena *arena, bool vargs, const SourcePosition &source_position)
        DECL_AND_INSTANTIATE(FunctionPrototype, prototype, node->prototype());
        
        Statement *body = nullptr;
        if (node->body()) {
            INSTANTIATE(body, node->body());
        }
        
        // FunctionDeclaration(base::Arena *arena, Decoration decoration, const String *name, FunctionPrototype *prototype,
        // bool is_reduce, const SourcePosition &source_position)
        auto full_name = original_ == node ? MakeFullName(node->name()) : node->name();
        auto fun = new (arena_) FunctionDeclaration(arena_, node->decoration(), full_name, prototype,
                                                    node->is_reduce(), node->source_position());
        fun->set_body(body);
        fun->set_access(node->access());
        fun->set_owns(node->owns());
        fun->set_package(node->package());
        fun->set_annotations(node->annotations());
        if (node == original()) {
            fun->set_original(node);
        }
        
        if (node == original()) {
            resolver_->FindOrInsert(fun->PackageName()->ToSlice(), fun->name()->ToSlice(), fun);
        }
        return Return(fun);
    }
private:
    int VisitVariableDeclaration(VariableDeclaration *node) override {
        // VariableDeclaration(base::Arena *arena, bool is_volatile, Constraint constraint,
        //                     const SourcePosition &source_position)
        auto copied = new (arena_) VariableDeclaration(arena_, node->is_volatile(), node->constraint(),
                                                       node->source_position());
        for (auto item : node->variables()) {
            // Item(base::Arena *arena, const String *identifier, class Type *type, const SourcePosition &source_position)
            Type *type = nullptr;
            if (item->type()) {
                INSTANTIATE(type, item->type());
            }
            auto other = new (arena_) VariableDeclaration::Item(arena_, copied, item->identifier(), type,
                                                                item->source_position());
            copied->mutable_variables()->push_back(other);
        }
        
        for (auto expr : node->initilaizers()) {
            DECL_AND_INSTANTIATE(Expression, it, expr);
            copied->mutable_initilaizers()->push_back(it);
        }
        return Return(copied);
    }
    
    int VisitBlock(Block *node) override {
        // Block(base::Arena *arena, const SourcePosition &source_position)
        auto copied = new (arena_) Block(arena_, node->source_position());
        for (auto stmt : node->statements()) {
            DECL_AND_INSTANTIATE(Statement, it, stmt);
            copied->mutable_statements()->push_back(it);
        }
        return Return(copied);
    }
    
    int VisitList(List *node) override {
        auto copied = new (arena_) List(arena_, node->source_position());
        for (auto stmt : node->expressions()) {
            DECL_AND_INSTANTIATE(Expression, it, stmt);
            copied->mutable_expressions()->push_back(it);
        }
        return Return(copied);
    }
    
    int VisitAssignment(Assignment *node) override {
        auto copied = new (arena_) Assignment(arena_, node->source_position());
        for (auto expr : node->lvals()) {
            DECL_AND_INSTANTIATE(Expression, it, expr);
            copied->mutable_lvals()->push_back(it);
        }
        for (auto expr : node->rvals()) {
            DECL_AND_INSTANTIATE(Expression, it, expr);
            copied->mutable_rvals()->push_back(it);
        }
        return Return(copied);
    }
    
    int VisitCalling(Calling *node) override {
        // Calling(base::Arena *arena, Expression *callee, const SourcePosition &source_position)
        DECL_AND_INSTANTIATE(Expression, callee, node->callee());
        auto copied = new (arena_) Calling(arena_, callee, node->source_position());
        for (auto arg : node->args()) {
            DECL_AND_INSTANTIATE(Expression, it, arg);
            copied->mutable_args()->push_back(it);
        }
        return Return(copied);
    }
    
    int VisitReturn(Return *node) override {
        auto copied = new (arena_) class Return(arena_, node->source_position());
        for (auto expr : node->returnning_vals()) {
            DECL_AND_INSTANTIATE(Expression, it, expr);
            node->mutable_returnning_vals()->push_back(it);
        }
        return Return(copied);
    }

    int VisitThrow(Throw *node) override {
        DECL_AND_INSTANTIATE(Expression, throwing_val, node->throwing_val());
        return Return(new (arena_) Throw(throwing_val, node->source_position()));
    }

    int VisitRunCoroutine(RunCoroutine *node) override {
        DECL_AND_INSTANTIATE(Calling, entry, node->entry());
        return Return(new (arena_) RunCoroutine(entry, node->source_position()));
    }

    int VisitWhileLoop(WhileLoop *node) override {
        Statement *init = nullptr;
        if (node->initializer()) {
            INSTANTIATE(init, node->initializer());
        }
        
        DECL_AND_INSTANTIATE(Expression, condition, node->condition());
        DECL_AND_INSTANTIATE(Block, body, node->body());

        return Return(new (arena_) WhileLoop(init, node->execute_first(), condition, body, node->source_position()));
    }
    
    int VisitUnlessLoop(UnlessLoop *node) override {
        Statement *init = nullptr;
        if (node->initializer()) {
            INSTANTIATE(init, node->initializer());
        }
        
        DECL_AND_INSTANTIATE(Expression, condition, node->condition());
        DECL_AND_INSTANTIATE(Block, body, node->body());
        
        return Return(new (arena_) UnlessLoop(init, node->execute_first(), condition, body, node->source_position()));
    }
    
    int VisitForeachLoop(ForeachLoop *node) override {
        DECL_AND_INSTANTIATE(Block, body, node->body());
        switch (node->iteration()) {
            case ForeachLoop::kIterator: {
                DECL_AND_INSTANTIATE(Expression, iterable, node->iterable());
                return Return(new (arena_) ForeachLoop(node->iterative_destination(), iterable, body,
                                                       node->source_position()));
            } break;
                
            case ForeachLoop::kOpenBound:
            case ForeachLoop::kCloseBound: {
                ForeachLoop::IntRange range {.lower = nullptr, .upper = nullptr, .close = node->range().close};
                INSTANTIATE(range.lower, node->range().lower);
                INSTANTIATE(range.upper, node->range().upper);
                return Return(new (arena_) ForeachLoop(node->iterative_destination(), range, body,
                                                       node->source_position()));
            } break;
                
            default:
                UNREACHABLE();
                break;
        }
        return -1;
    }
    
    int VisitStringTemplate(StringTemplate *node) override {
        auto copied = new (arena_) StringTemplate(arena_, node->source_position());
        for (auto part : node->parts()) {
            DECL_AND_INSTANTIATE(Expression, it, part);
            copied->mutable_parts()->push_back(it);
        }
        return Return(copied);
    }
    
    int VisitOr(Or *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Or(lhs, rhs, node->source_position()));
    }
    
    int VisitAdd(Add *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Add(lhs, rhs, node->source_position()));
    }
    
    int VisitAnd(And *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) And(lhs, rhs, node->source_position()));
    }

    int VisitDiv(Div *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Div(lhs, rhs, node->source_position()));
    }
    
    int VisitDot(Dot *node) override {
        DECL_AND_INSTANTIATE(Expression, primary, node->primary());
        return Return(new (arena_) Dot(primary, node->field(), node->source_position()));
    }
    
    int VisitMod(Mod *node) {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Mod(lhs, rhs, node->source_position()));
    }
    
    int VisitMul(Mul *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Mul(lhs, rhs, node->source_position()));
    }

    int VisitNot(Not *node) override {
        DECL_AND_INSTANTIATE(Expression, opd, node->operand());
        return Return(new (arena_) Not(opd, node->source_position()));
    }

    int VisitSub(Sub *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Sub(lhs, rhs, node->source_position()));
    }

    int VisitLess(Less *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Less(lhs, rhs, node->source_position()));
    }

    int VisitLessEqual(LessEqual *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) LessEqual(lhs, rhs, node->source_position()));
    }

    int VisitGreater(Greater *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Greater(lhs, rhs, node->source_position()));
    }

    int VisitGreaterEqual(GreaterEqual *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) GreaterEqual(lhs, rhs, node->source_position()));
    }

    int VisitEqual(Equal *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Equal(lhs, rhs, node->source_position()));
    }

    int VisitNotEqual(NotEqual *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) NotEqual(lhs, rhs, node->source_position()));
    }

    int VisitRecv(Recv *node) override {
        DECL_AND_INSTANTIATE(Expression, opd, node->operand());
        return Return(new (arena_) Recv(opd, node->source_position()));
    }

    int VisitSend(Send *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) Send(lhs, rhs, node->source_position()));
    }

    int VisitNegative(Negative *node) override {
        DECL_AND_INSTANTIATE(Expression, opd, node->operand());
        return Return(new (arena_) Negative(opd, node->source_position()));
    }

    int VisitBitwiseOr(BitwiseOr *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) BitwiseOr(lhs, rhs, node->source_position()));
    }

    int VisitBitwiseAnd(BitwiseAnd *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) BitwiseAnd(lhs, rhs, node->source_position()));
    }

    int VisitBitwiseShl(BitwiseShl *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) BitwiseShl(lhs, rhs, node->source_position()));
    }

    int VisitBitwiseShr(BitwiseShr *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) BitwiseShr(lhs, rhs, node->source_position()));
    }

    int VisitBitwiseXor(BitwiseXor *node) override {
        DECL_AND_INSTANTIATE(Expression, lhs, node->lhs());
        DECL_AND_INSTANTIATE(Expression, rhs, node->rhs());
        return Return(new (arena_) BitwiseXor(lhs, rhs, node->source_position()));
    }

    int VisitBitwiseNegative(BitwiseNegative *node) override {
        DECL_AND_INSTANTIATE(Expression, opd, node->operand());
        return Return(new (arena_) BitwiseNegative(opd, node->source_position()));
    }

    int VisitIdentifier(Identifier *node) override { return Return(node); }
    
    int VisitCasting(Casting *node) override {
        DECL_AND_INSTANTIATE(Expression, source, node->source());
        DECL_AND_INSTANTIATE(Type, dest, node->destination());
        return Return(new (arena_) Casting(source, dest, node->source_position()));
    }
    
    int VisitTesting(Testing *node) override {
        DECL_AND_INSTANTIATE(Expression, source, node->source());
        DECL_AND_INSTANTIATE(Type, dest, node->destination());
        return Return(new (arena_) Testing(source, dest, node->source_position()));
    }
    
    int VisitIndexedGet(IndexedGet *node) override {
        DECL_AND_INSTANTIATE(Expression, primary, node->primary());
        auto expr = new (arena_) IndexedGet(arena_, primary, node->source_position());
        for (auto index : node->indexs()) {
            DECL_AND_INSTANTIATE(Expression, idx, index);
            expr->mutable_indexs()->push_back(idx);
        }
        return Return(expr);
    }
    
    int VisitAssertedGet(AssertedGet *node) override {
        DECL_AND_INSTANTIATE(Expression, opd, node->operand());
        return Return(new (arena_) AssertedGet(opd, node->source_position()));
    }
    
    int VisitIfExpression(IfExpression *node) override {
        Statement *init = nullptr;
        if (node->initializer()) {
            INSTANTIATE(init, node->initializer());
        }
        DECL_AND_INSTANTIATE(Expression, condition, node->condition());
        DECL_AND_INSTANTIATE(Statement, then_clause, node->then_clause());
        Statement *else_clause = nullptr;
        if (node->else_clause()) {
            INSTANTIATE(else_clause, node->else_clause());
        }
        return Return(new (arena_) IfExpression(arena_, init, condition, then_clause, else_clause,
                                                node->source_position()));
    }
    
    int VisitLambdaLiteral(LambdaLiteral *node) override {
        DECL_AND_INSTANTIATE(Block, body, node->body());
        DECL_AND_INSTANTIATE(FunctionPrototype, prototype, node->prototype());
        return Return(new (arena_) LambdaLiteral(prototype, body, node->source_position()));
    }
    
    int VisitWhenExpression(WhenExpression *node) override { UNREACHABLE(); }
    int VisitArrayInitializer(ArrayInitializer *node) override { UNREACHABLE(); }
    int VisitObjectDeclaration(ObjectDeclaration *node) override { UNREACHABLE(); }
    
    int VisitTryCatchExpression(TryCatchExpression *node) override { UNREACHABLE(); }

    int VisitInstantiation(Instantiation *node) override {
        std::unique_ptr<Type *[]> types;
        for (size_t i = 0; i < node->generic_args_size(); i++) {
            if (types[i] = TypeLink(node->generic_arg(i)); !types[i]) {
                return -1;
            }
        }
        AstNode *ast = nullptr;
        bool not_found = false;
        if (node->primary()->IsIdentifier()) {
            auto prefix = "";
            auto name = node->primary()->AsIdentifier()->name()->ToSlice();
            ast = Instantiate(node->source_position(), prefix, name, &types[0], node->generic_args_size(), &not_found);
        } else {
            assert(node->primary()->IsDot());
            auto dot = node->primary()->AsDot();
            if (!dot->primary()->IsIdentifier()) {
                goto clone;
            }
            auto prefix = dot->primary()->AsIdentifier()->name()->ToSlice();
            auto name = dot->field()->ToSlice();
            ast = Instantiate(node->source_position(), prefix, name, &types[0], node->generic_args_size(), &not_found);
        }
        if (not_found) {
            assert(ast == nullptr);
            goto clone;
        }
        if (!ast) {
            return -1;
        }

        if (node->primary()->IsIdentifier()) {
            //auto id = new (arena_) Identifier()
            auto name = BuildFullName(node->primary()->AsIdentifier()->name(), node->generic_args_size(), &types[0]);
            auto primary = new (arena_) Identifier(String::New(arena_, name), node->primary()->source_position());
            auto copied = new (arena_) Instantiation(arena_, primary, node->source_position());
            for (size_t i = 0; i < node->generic_args_size(); i++) {
                copied->mutable_generic_args()->push_back(types[i]);
            }
        } else {
            assert(node->primary()->IsDot());
            auto dot = node->primary()->AsDot();
            assert(dot->primary()->IsIdentifier());
            auto name = BuildFullName(dot->field(), node->generic_args_size(), &types[0]);
            auto primary = new (arena_) Dot(dot->primary(), String::New(arena_, name), dot->source_position());
            auto copied = new (arena_) Instantiation(arena_, primary, node->source_position());
            for (size_t i = 0; i < node->generic_args_size(); i++) {
                copied->mutable_generic_args()->push_back(types[i]);
            }
        }
        
        clone: {
            AstNode *it = nullptr;
            if (it = Instantiate(node->primary()); !it) {
                return -1;
            }
            auto copied = new (arena_) Instantiation(arena_, static_cast<Expression *>(it), node->source_position());
            for (size_t i = 0; i < node->generic_args_size(); i++) {
                copied->mutable_generic_args()->push_back(types[i]);
            }
            return Return(copied);
        }
    }
    
    int VisitOptionLiteral(OptionLiteral *node) override {
        if (node->is_some()) {
            DECL_AND_INSTANTIATE(Expression, value, node->value());
            return Return(new (arena_) OptionLiteral(arena_, value, node->source_position()));
        } else {
            return Return(new (arena_) OptionLiteral(arena_, nullptr, node->source_position()));
        }
    }

    int VisitBreak(Break *node) override { return Return(node); }
    int VisitContinue(Continue *node) override { return Return(node); }
    int VisitF32Literal(F32Literal *node) override { return Return(node); }
    int VisitF64Literal(F64Literal *node) override { return Return(node); }
    int VisitI64Literal(I64Literal *node) override { return Return(node); }
    int VisitIntLiteral(IntLiteral *node) override { return Return(node); }
    int VisitUIntLiteral(UIntLiteral *node) override { return Return(node); }
    int VisitU64Literal(U64Literal *node) override { return Return(node); }
    int VisitBoolLiteral(BoolLiteral *node) override { return Return(node); }
    int VisitUnitLiteral(UnitLiteral *node) override { return Return(node); }
    int VisitEmptyLiteral(EmptyLiteral *node) override { return Return(node); }
    int VisitStringLiteral(StringLiteral *node) override { return Return(node); }
    int VisitCharLiteral(CharLiteral *node) override { return Return(node); }
    
    int VisitChannelInitializer(ChannelInitializer *node) override {
        auto ty = DCHECK_NOTNULL(node->type()->AsChannelType());
        auto copied_ty = new (arena_) ChannelType(arena_, ty->ability(), ty->element_type(), ty->source_position());
        copied_ty = down_cast<ChannelType>(TypeLink(copied_ty));
        if (!copied_ty) {
            return -1;
        }
        auto copied = new (arena_) ChannelInitializer(copied_ty, node->capacity(), node->source_position());
        return Return(copied);
    }
    
    int VisitPackage(Package *node) override { UNREACHABLE(); }
    int VisitFileUnit(FileUnit *node) override { UNREACHABLE(); }
    int VisitAnnotationDefinition(AnnotationDefinition *node) override { UNREACHABLE(); }
    int VisitAnnotationDeclaration(AnnotationDeclaration *node) override { UNREACHABLE(); }
    int VisitAnnotation(Annotation *node) override { UNREACHABLE(); }
    
    int ProcessIncompletableDefinition(const IncompletableDefinition *node, IncompletableDefinition *copied,
                                       IncompletableDefinition **base_of) {
        for (auto param : node->parameters()) {
            if (param.field_declaration) {
                copied->mutable_parameters()->push_back(param);
            } else {
                StructDefinition::Parameter other;
                other.field_declaration = false;
                INSTANTIATE(other.as_parameter, param.as_parameter);
                copied->mutable_parameters()->push_back(other);
            }
        }
        
        for (auto field : node->fields()) {
            StructDefinition::Field other;
            other.as_constructor = field.as_constructor;
            other.in_constructor = field.in_constructor;
            INSTANTIATE(other.declaration, field.declaration);
            copied->mutable_fields()->push_back(other);
        }
        
        for (auto method : node->methods()) {
            DECL_AND_INSTANTIATE(FunctionDeclaration, it, method);
            copied->mutable_methods()->push_back(it);
        }
        
        if (node->super_calling()) {
            if (auto inst = node->super_calling()->callee()->AsInstantiation()) {
                //auto inst = node->super_calling()->AsInstantiation();
                std::unique_ptr<Type *[]> types(new Type *[inst->generic_args_size()]);
                for (size_t i = 0; i < inst->generic_args_size(); i++) {
                    if (types[i] = TypeLink(inst->generic_arg(i)); !types[i]) {
                        return -1;
                    }
                }
                
                Statement *base = nullptr;
                if (inst->primary()->IsIdentifier()) {
                    base = Instantiate(inst->source_position(), "",
                                       inst->primary()->AsIdentifier()->name()->ToSlice(),
                                       &types[0],
                                       inst->generic_args_size());
                } else {
                    assert(inst->primary()->IsDot());
                    auto dot = inst->primary()->AsDot();
                    assert(dot->primary()->IsIdentifier());
                    base = Instantiate(inst->source_position(),
                                       dot->primary()->AsIdentifier()->name()->ToSlice(),
                                       dot->field()->ToSlice(),
                                       &types[0],
                                       inst->generic_args_size());
                }
                if (!base) {
                    return -1;
                }
                if (!base->IsClassDefinition() && !base->IsStructDefinition()) {
                    Feedback()->Printf(inst->source_position(), "Base is not class or struct");
                    return -1;
                }
                *base_of = down_cast<IncompletableDefinition>(base);
            }
        }
        return 0;
    }
    
    template<class T>
    int PrepareTemplate(T *node) {
        if (node->generic_params().empty()) {
            Feedback()->Printf(node->source_position(), "%s is not a generics type",
                               node->owns() ? node->FullName().c_str() : node->name()->data());
            return -1;
        }
        if (argc_ != node->generic_params_size()) {
            Feedback()->Printf(node->source_position(), "Different generics parameters, %zd vs %zd ",
                               node->FullName().c_str(), node->generic_params_size(), argc_);
            return -1;
        }
        for (size_t i = 0; i < argc_; i++) {
            Definition *def = nullptr;
            switch (argv_[i]->category()) {
                case Type::kStruct:
                    def = argv_[i]->AsStructType()->definition();
                    break;
                case Type::kClass:
                    def = argv_[i]->AsClassType()->definition();
                    break;
                case Type::kInterface:
                    def = argv_[i]->AsInterfaceType()->definition();
                    break;
                default:
                    break;
            }
            if (def != nullptr && static_cast<Statement *>(def->original()) == node) {
                Feedback()->Printf(argv_[i]->source_position(), "Recursive generics type: %s<%s>",
                                   node->FullName().c_str(),
                                   def->FullName().c_str());
                return -1;
            }
            args_[node->generic_param(i)->name()->ToSlice()] = argv_[i];
            //node->generic_param(i)->set_instantiation(argv_[i]);
        }
        return 0;
    }
    
//    int PrepareDefinition(Definition *node) {
//        if (node->generic_params().empty()) {
//            Feedback()->Printf(node->source_position(), "%s is not a generics type", node->FullName().c_str());
//            return -1;
//        }
//        if (argc_ != node->generic_params_size()) {
//            Feedback()->Printf(node->source_position(), "Different generics parameters, %zd vs %zd ",
//                               node->FullName().c_str(), node->generic_params_size(), argc_);
//            return -1;
//        }
//        for (size_t i = 0; i < argc_; i++) {
//            Definition *def = nullptr;
//            switch (argv_[i]->category()) {
//                case Type::kStruct:
//                    def = argv_[i]->AsStructType()->definition();
//                    break;
//                case Type::kClass:
//                    def = argv_[i]->AsClassType()->definition();
//                    break;
//                case Type::kInterface:
//                    def = argv_[i]->AsInterfaceType()->definition();
//                    break;
//                default:
//                    break;
//            }
//            if (def != nullptr && def->original() == node) {
//                Feedback()->Printf(argv_[i]->source_position(), "Recursive generics type: %s<%s>",
//                                   node->FullName().c_str(),
//                                   def->FullName().c_str());
//                return -1;
//            }
//            args_[node->generic_param(i)->name()->ToSlice()] = argv_[i];
//            //node->generic_param(i)->set_instantiation(argv_[i]);
//        }
//        return 0;
//    }
    
    Statement *Instantiate(const SourcePosition &location,
                           std::string_view prefix,
                           std::string_view name,
                           Type *argv[],
                           size_t argc,
                           bool *not_found = nullptr) {
        Statement *ast = resolver_->Find(prefix, name);
        if (!ast) {
            if (!not_found) {
                Feedback()->Printf(location, "Symbol `%s' not found", name.data());
            } else {
                *not_found = true;
            }
            return nullptr;
        }

        if (!Definition::Is(ast)) {
            Feedback()->Printf(location, "Symbol `%s.%s' is not class/struct/interface", prefix.data(), name.data());
            return nullptr;
        }
        auto def = down_cast<Definition>(ast);
        if (!def->generic_params().empty()) {
            auto pkg = def->PackageName()->ToSlice();
            ast = resolver_->Find(pkg, BuildFullName(def->name(), argc, argv)); // Find exists
            //printf("%s %p\n", BuildFullName(def->name(), argc, argv).c_str(), ast);
            if (!ast) {
                //auto actual_name = String::New(arena_, BuildFullName(def->name(), argc, argv));
                if (status_ = GenericsInstantiating::Instantiate(nullptr, def, arena_, feedback_,
                                                                 std::move(resolver_), argc, argv, &ast);
                    status().fail()) {
                    return nullptr;
                }
                resolver_->FindOrInsert(pkg, BuildFullName(def->name(), argc, argv), ast);
            }
        }
        return ast;
    }
    
    int Return(AstNode *ast) {
        if (ast) {
            results_.push(ast);
            return 1;
        }
        return -1;
    }
    
    AstNode *Instantiate(AstNode *ast) {
        int rv = ast->Accept(this);
        if (rv < 0 || status().fail()) {
            return nullptr;
        }
        auto result = results_.top();
        results_.pop();
        return result;
    }

    Type *Instantiate(Type *type) {
        switch (type->category()) {
            case Type::kPrimary: {
                if (type->primary_type() == Type::kType_symbol) {
                    //assert(type->primary_type() == Type::kType_symbol);
                    auto copied = new (arena_) Type(arena_, type->identifier(), type->source_position());
                    for (auto arg : type->generic_args()) {
                        if (auto it = Instantiate(arg); !it) {
                            return nullptr;
                        } else {
                            copied->mutable_generic_args()->push_back(it);
                        }
                    }
                    return TypeLink(copied);
                }
            } return type;
                
            case Type::kArray: {
                auto src = DCHECK_NOTNULL(type->AsArrayType());
                Type *it = nullptr;
                if (it = Instantiate(src->element_type()); !it) {
                    return nullptr;
                }
                return new (arena_) ArrayType(arena_, it, src->dimension_count(), src->source_position());
            } break;
                
            case Type::kChannel: {
                auto src = DCHECK_NOTNULL(type->AsChannelType());
                Type *it = nullptr;
                if (it = Instantiate(src->element_type()); !it) {
                    return nullptr;
                }
                return new (arena_) ChannelType(arena_, src->ability(), it, src->source_position());
            } break;
                
            case Type::kFunction: {
                auto src = DCHECK_NOTNULL(type->AsFunctionPrototype());
                auto copied = new (arena_) FunctionPrototype(arena_, src->vargs(), src->source_position());
                for (auto param : src->params()) {
                    auto item = static_cast<VariableDeclaration::Item *>(param);
                    auto it = Instantiate(item->type());
                    if (!it) {
                        return nullptr;
                    }
                    auto other = new (arena_) VariableDeclaration::Item(arena_, nullptr, item->identifier(), it,
                                                                        item->source_position());
                    copied->mutable_params()->push_back(other);
                }
                
                for (auto returning : src->return_types()) {
                    auto it = Instantiate(returning);
                    if (!it) {
                        return nullptr;
                    }
                    copied->mutable_return_types()->push_back(it);
                }
                return copied;
            } break;
                
            default:
                break;
        }
        return type;
    }
    
    Type *TypeLink(Type *type) {
        using std::placeholders::_1;
        using std::placeholders::_2;
        return type->Link(std::bind(&GenericsInstantiatingVisitor::TypeLinker, this, _1, _2));
    }
    
    Type *TypeLinker(const Symbol *name, Type *host) {
        if (!name->prefix_name()) {
            auto iter = args_.find(name->name()->ToSlice());
            if (iter != args_.end()) {
                return iter->second;
            }
        }

        Statement *ast = Instantiate(name->source_position(),
                                     !name->prefix_name()
                                     ? "" : name->prefix_name()->ToSlice(), name->name()->ToSlice(),
                                     &(*host->mutable_generic_args())[0],
                                     host->generic_args_size());
        if (!ast) {
            return nullptr;
        }
        
        switch (ast->kind()) {
            case Node::kClassDefinition:
                return new (arena_) ClassType(arena_, ast->AsClassDefinition(), name->source_position());
            case Node::kStructDefinition:
                return new (arena_) StructType(arena_, ast->AsStructDefinition(), name->source_position());
            case Node::kInterfaceDefinition:
                return new (arena_) InterfaceType(arena_, ast->AsInterfaceDefinition(), name->source_position());
            default:
                break;
        }
        UNREACHABLE();
    }
    
    const String *MakeFullName(const String *name) {
        return !actual_name_ ? String::New(arena_, BuildFullName(name, argc_, argv_)) : actual_name_;
    }
    
    std::string BuildFullName(const String *name, size_t argc, Type **types) {
        std::string buf(name->ToString());
        buf.append("<");
        for (size_t i = 0; i < argc; i++) {
            if (i > 0) {
                buf.append(",");
            }
            buf.append(types[i]->ToString());
        }
        buf.append(">");
        return buf;
    }
    
    std::string_view GetPackageName(const String *full_name) {
        auto p = strchr(full_name->data(), '.');
        if (!p) {
            return "";
        }
        return std::string_view(full_name->data(), p - full_name->data());
    }
    
    SyntaxFeedback *FeedbackWith(const char *file, int line) {
        status_ = base::Status::Corruption(file, line, "Generics instantiating fail!");
        return feedback_;
    }
    
    base::Arena *const arena_;
    SyntaxFeedback *const feedback_;
    GenericsInstantiating::Resolver *const resolver_;
    Statement *const original_ = nullptr;
    const String *const actual_name_;
    const size_t argc_;
    Type **argv_;
    std::map<std::string_view, Type *> args_;
    std::stack<AstNode *> results_;
    base::Status status_ = base::Status::OK();
}; // class GenericsInstantiatingVisitor


base::Status GenericsInstantiating::Instantiate(const String *actual_name,
                                                Statement *def,
                                                base::Arena *arena,
                                                SyntaxFeedback *feedback,
                                                Resolver *resolver,
                                                size_t argc,
                                                Type **argv,
                                                Statement **inst) {
    GenericsInstantiatingVisitor visitor(actual_name, arena, feedback, resolver, def, argc, argv);
    resolver->Enter(def);
    def->Accept(&visitor);
    resolver->Exit(def);
    if (visitor.status().fail()) {
        return visitor.status();
    }
    *inst = visitor.result();
    return base::Status::OK();
}

} // namespace cpl

} // namespace yalx
