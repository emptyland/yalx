#pragma once
#ifndef YALX_COMPILER_NODE_H_
#define YALX_COMPILER_NODE_H_

#include "compiler/source-position.h"
#include "base/arena.h"

namespace yalx {
namespace base {
class ArenaString;
} // namespace base
namespace cpl {

// Node
//    +: il
//    +- FileUnit
//    +- Statement
//        +- Block
//        +- BreakStatement
//        +- ContinueStatement
//        +- ReturnStatement
//        +- ThrowStatement
//        +- WhileStatement
//        +- ForEachStatement
//        +- ForStepStatement
//        +- Declaration
//            +- VariableDeclaration
//            +- FunctionDeclaration
//            +- ObjectDeclaration
//        +- Definition
//            +- InterfaceDefinition
//            +- ClassDefinition
//            +- AnnotationDefinition
//        +- Expression
//            +- Identifier
//            +- Literal
//                +- UnitLiteral
//                +- ActualLiteral<T>
//                +- LambdaLiteral
//                    +- IntLiteral
//                    +- UIntLiteral
//                    +- I64Literal
//                    +- U64Literal
//                    +- BoolLiteral
//                    +- StringLiteral
//            +- ExpressionWithOperands<N>
//                +- BinaryExpression
//                    +- Add
//                    +- Sub
//                    +- Mul
//                    +- Div
//                    +- Mod
//                    +- Equal
//                    +- NotEqual
//                    +- Less
//                    +- LessEqual
//                    +- Greater
//                    +- GreaterEqual
//                    +- And
//                    +- Or
//                    +- BitwiseAnd
//                    +- BitwiseOr
//                    +- BitwiseXor
//                    +- ChannelWrite
//                    +- IndexedGet
//                +- UnaryExpression
//                    +- Negative
//                    +- Not
//                    +- BitwiseNegative
//                    +- ChannelRead
//            +- Dot
//            +- Casting
//            +- Calling
//            +- IfExpression
//            +- WhenExpression
//            +- TryCatchExpression
//            +- ChannelInitializer
//            +- ArrayInitializer
//            +- ArrayDimension
//    +- Annotation
//    +- Type
//        +- ArrayType
//        +- ClassType
//        +- InterfaceType
//        +- FunctionPrototype
//    +- Operand
//        +- Argument
//        +- StackAllocate
//        +- HeapAllocate
//        +- StackLoad
//        +- StackStore
//        +- GlobalLoad
//        +- GlobalStore
//        +- CallRuntime
//        +- CallDirect
//        +- CallIndirect
//        +- CallVirtual
//        +- Arithmetic<N>
//            +- I8{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U8{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- I16{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U16{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- I32{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U32{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- I64{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Sar}
//            +- U64{Add/Sub/Mul/Div/Mod/And/Or/Not/Xor/Inv/Shl/Shr}
//            +- F32{Add/Sub/Mul/Div}
//            +- F64{Add/Sub/Mul/Div}

#define DECLARE_ALL_NODES(V) \
    DECLARE_AST_NODES(V) \
    DECLARE_TYPE_NODES(V)

#define DECLARE_AST_NODES(V) \
    V(FileUnit) \
    V(Block) \
    V(BreakStatement) \
    V(ContinueStatement) \
    V(VariableDeclaration) \
    V(ClassDefinition) \
    V(StructDefinition) \
    V(InterfaceDefinition) \
    V(AnnotationDefinition) \
    V(AnnotationDeclaration) \
    V(Annotation) \
    V(Identifier) \
    V(UnitLiteral) \
    V(EmptyLiteral) \
    V(IntLiteral) \
    V(UIntLiteral) \
    V(I64Literal) \
    V(U64Literal) \
    V(F32Literal) \
    V(F64Literal) \
    V(BoolLiteral) \
    V(StringLiteral) \
    V(LambdaLiteral) \
    V(ArrayInitializer) \
    V(Negative) \
    V(Add) \
    V(Sub) \
    V(Mul) \
    V(Div) \
    V(Mod) \
    V(Equal) \
    V(NotEqual) \
    V(Less) \
    V(LessEqual) \
    V(Greater) \
    V(GreaterEqual) \
    V(And) \
    V(Or) \
    V(Not) \
    V(BitwiseAnd) \
    V(BitwiseOr) \
    V(BitwiseXor) \
    V(BitwiseNegative) \
    V(BitwiseShl) \
    V(BitwiseShr) \
    V(Recv) \
    V(Send) \
    V(IndexedGet) \
    V(Dot)

#define DECLARE_TYPE_NODES(V) \
    V(Type) \
    V(ArrayType) \
    V(ChannelType) \
    V(ClassType) \
    V(StructType) \
    V(InterfaceType) \
    V(FunctionPrototype)

#define DEFINE_PREDECL_CLASSES(name) class name;
DECLARE_ALL_NODES(DEFINE_PREDECL_CLASSES)
#undef DEFINE_PREDECL_CLASSES


using String = base::ArenaString;
class Literal;
class Expression;
class Statement;

class Node : public base::ArenaObject {
public:
    enum Kind {
#define DEFINE_ENUM(name) k##name,
        DECLARE_ALL_NODES(DEFINE_ENUM)
#undef DEFINE_ENUM
        kMaxKinds,
    }; // enum Kind

//#define DEFINE_METHODS(name) \
//    bool Is##name() const { return kind() == k##name; } \
//    name *As##name() { return !Is##name() ? nullptr : static_cast<name *>(this); } \
//    const name *As##name() const { return !Is##name() ? nullptr : static_cast<const name *>(this); }
#define DEFINE_METHODS(name) \
    bool Is##name() const { return kind() == k##name; } \
    inline name *As##name(); \
    inline const name *As##name() const; 
    DECLARE_ALL_NODES(DEFINE_METHODS)
#undef DEFINE_METHODS

    DEF_VAL_GETTER(Kind, kind);
    DEF_VAL_PROP_RM(SourcePosition, source_position);
    
    DISALLOW_IMPLICIT_CONSTRUCTORS(Node);
protected:
    Node(Kind kind, const SourcePosition &source_position): kind_(kind), source_position_(source_position) {}
    
private:
    Kind kind_;
    SourcePosition source_position_;
}; // class Node


class Symbol : public Node {
public:
    Symbol(const String *name, const SourcePosition &source_position = {0, 0})
        : Symbol(nullptr, name, source_position) {}

    Symbol(const String *prefix_name, const String *name, const SourcePosition &source_position = {0, 0})
        : Node(kMaxKinds, source_position)
        , prefix_name_(prefix_name)
        , name_(name) {}

    DEF_PTR_GETTER(const String, prefix_name);
    DEF_PTR_GETTER(const String, name);
private:
    const String *prefix_name_;
    const String *name_;
}; // class Symbol

class Visitation {
    // TODO:
}; // class Visitation

} // namespace cpl

} // namespace yalx

#endif // YALX_COMPILER_NODE_H_
