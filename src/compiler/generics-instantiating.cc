#include "compiler/generics-instantiating.h"
#include "compiler/syntax-feedback.h"
#include "compiler/scope.h"
#include "compiler/ast.h"
#include <stack>

namespace yalx {

namespace cpl {

#define Feedback() FeedbackWith(__FILE__, __LINE__)

class GenericsInstantiatingVisitor : public AstVisitor {
public:
    GenericsInstantiatingVisitor(base::Arena *arena, SyntaxFeedback *feedback,
                                 GenericsInstantiating::Resolver *resolver,
                                 Statement *original,
                                 const size_t argc, Type **argv)
        : arena_(DCHECK_NOTNULL(arena))
        , feedback_(feedback)
        , resolver_(resolver)
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
        if (PrepareDefinition(node) < 0){
            return -1;
        }

        // InterfaceDefinition(base::Arena *arena, const String *name, const SourcePosition &source_position);
        auto copied = new (arena_) InterfaceDefinition(arena_, MakeFullName(node->name()), node->source_position());
        copied->set_owns(node->owns());
        copied->set_package(node->package());
        
        resolver_->FindOrInsert(copied->PackageName()->ToSlice(), copied->name()->ToSlice(), copied); // Insert first
        
        for (auto method : node->methods()) {
            if (auto it = Instantiate(method); !it) {
                return -1;
            } else {
                copied->mutable_methods()->push_back(it->AsFunctionDeclaration());
            }
        }

        return Return(copied);
    }
    
    int VisitFunctionDeclaration(FunctionDeclaration *node) override {
        // FunctionPrototype(base::Arena *arena, bool vargs, const SourcePosition &source_position)
        auto prototype = new (arena_) FunctionPrototype(arena_, node->prototype()->vargs(),
                                                        node->prototype()->source_position());
        for (auto param : node->prototype()->params()) {
            auto item = static_cast<VariableDeclaration::Item *>(param);
            
            // Item(base::Arena *arena, const String *identifier, class Type *type, const SourcePosition &source_position)
            auto it = TypeLink(item->type());
            if (!it) {
                return -1;
            }
            auto copied = new (arena_) VariableDeclaration::Item(arena_, item->identifier(), it,
                                                                 item->source_position());
            prototype->mutable_params()->push_back(copied);
        }
        
        for (auto returning : node->prototype()->return_types()) {
            auto it = TypeLink(returning);
            if (!it) {
                return -1;
            }
            prototype->mutable_return_types()->push_back(it);
        }
        
        Statement *body = nullptr;
        if (node->body()) {
            if (body = static_cast<Statement *>(Instantiate(node->body())); !body) {
                return -1;
            }
        }
        
        // FunctionDeclaration(base::Arena *arena, Decoration decoration, const String *name, FunctionPrototype *prototype,
        // bool is_reduce, const SourcePosition &source_position)
        auto fun = new (arena_) FunctionDeclaration(arena_, node->decoration(), node->name(), prototype,
                                                    node->is_reduce(), node->source_position());
        fun->set_body(body);
        fun->set_owns(node->owns());
        fun->set_package(node->package());
        return Return(fun);
    }
private:
    int VisitPackage(Package *node) override { UNREACHABLE(); }
    int VisitFileUnit(FileUnit *node) override { UNREACHABLE(); }
    int VisitBlock(Block *node) override { UNREACHABLE(); }
    int VisitList(List *node) override { UNREACHABLE(); }
    int VisitVariableDeclaration(VariableDeclaration *node) override { UNREACHABLE(); }
    int VisitAssignment(Assignment *node) override { UNREACHABLE(); }
    int VisitStructDefinition(StructDefinition *node) override { UNREACHABLE(); }
    int VisitClassDefinition(ClassDefinition *node) override { UNREACHABLE(); }
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
    int VisitUIntLiteral(UIntLiteral *node) override { UNREACHABLE(); }
    int VisitTryCatchExpression(TryCatchExpression *node) override { UNREACHABLE(); }
    
private:
    int PrepareDefinition(Definition *node) {
        if (node->generic_params().empty()) {
            Feedback()->Printf(node->source_position(), "%s is not a generics type", node->FullName().c_str());
            return -1;
        }
        if (argc_ != node->generic_params_size()) {
            Feedback()->Printf(node->source_position(), "Different generics parameters, %zd vs %zd ",
                               node->FullName().c_str(), node->generic_params_size(), argc_);
            return -1;
        }
        for (size_t i = 0; i < argc_; i++) {
            args_[node->generic_param(i)->name()->ToSlice()] = argv_[i];
            //node->generic_param(i)->set_instantiation(argv_[i]);
        }
        return 0;
    }
    
    Type *TypeLink(Type *type) {
        return type->Link(std::bind(&GenericsInstantiatingVisitor::TypeLinker, this, std::placeholders::_1,
                                    std::placeholders::_2));
    }
    
    Type *TypeLinker(const Symbol *name, Type *host) {
        Statement *ast = nullptr;
        if (!name->prefix_name()) {
            auto iter = args_.find(name->name()->ToSlice());
            if (iter != args_.end()) {
                return iter->second;
            }
            ast = resolver_->Find("", name->name()->ToSlice());
        } else {
            ast = resolver_->Find(name->prefix_name()->ToSlice(), name->name()->ToSlice());
        }
        if (!ast) {
            Feedback()->Printf(name->source_position(), "Symbol %s not found", name->ToString().c_str());
            return nullptr;
        }
        if (!Definition::Is(ast)) {
            Feedback()->Printf(name->source_position(), "Symbol %s is not class/struct/interface",
                               name->ToString().c_str());
            return nullptr;
        }
        auto def = down_cast<Definition>(ast);
        if (!def->generic_params().empty()) {
            auto pkg = def->PackageName()->ToSlice();
            auto argc = host->generic_args_size();
            auto argv = &host->mutable_generic_args()->at(0);
            ast = resolver_->Find(pkg, BuildFullName(def->name(), argc, argv)); // Find exists
            if (!ast) {
                if (status_ = GenericsInstantiating::Instantiate(def, arena_, feedback_, std::move(resolver_), argc,
                                                                 argv, &ast);
                    status().fail()){
                    return nullptr;
                }
                resolver_->FindOrInsert(pkg, BuildFullName(def->name(), argc, argv), ast);
            }
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
    
    const String *MakeFullName(const String *name) { return String::New(arena_, BuildFullName(name, argc_, argv_)); }
    
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
    GenericsInstantiating::Resolver *resolver_;
    Statement *original_ = nullptr;
    const size_t argc_;
    Type **argv_;
    std::map<std::string_view, Type *> args_;
    std::stack<AstNode *> results_;
    base::Status status_ = base::Status::OK();
}; // class GenericsInstantiatingVisitor


base::Status GenericsInstantiating::Instantiate(Statement *def,
                                                base::Arena *arena,
                                                SyntaxFeedback *feedback,
                                                Resolver *resolver,
                                                size_t argc,
                                                Type **argv,
                                                Statement **inst) {
    GenericsInstantiatingVisitor visitor(arena, feedback, resolver, def, argc, argv);
    def->Accept(&visitor);
    if (visitor.status().fail()) {
        return visitor.status();
    }
    *inst = visitor.result();
    return base::Status::OK();
}

} // namespace cpl

} // namespace yalx
