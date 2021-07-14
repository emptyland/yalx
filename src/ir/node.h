#pragma once
#ifndef YALX_IR_NODE_H_
#define YALX_IR_NODE_H_

#include "ir/operator.h"
#include "ir/type.h"
#include "base/checking.h"
#include "base/arena-utils.h"
#include "base/arena.h"
#include <memory>

namespace yalx {
namespace base {
class ArenaString;
} // namespace base
namespace ir {

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

using String = base::ArenaString;



#define DECLARE_IR_KINDS(V) \
    V(Module) \
    V(Function) \
    V(BasicBlock) \
    V(Value)

#define DEFINE_PREDECL(name) class name;
    DECLARE_IR_KINDS(DEFINE_PREDECL)
#undef DEFINE_PREDECL

class Node : public base::ArenaObject {
public:
    enum Kind {
    #define DEFINE_ENUM(name) k##name,
        DECLARE_IR_KINDS(DEFINE_ENUM)
    #undef DEFINE_ENUM
    };
    
#define DECLARE_METHOD(name) \
    inline name *As##name(); \
    inline const name *As##name() const; \
    bool Is##name() const { return kind_ == k##name; }
    DECLARE_IR_KINDS(DECLARE_METHOD)
#undef DECLARE_METHOD
protected:
    Node(Kind kind): kind_(kind) {}
    
private:
    Kind const kind_;
    int mark_ = 0;
}; // class Node


class Module : public Node {
public:
    
}; // class Module

class Function : public Node {
public:
    
private:
    Module *owns_;
    base::ArenaVector<Value *> paramaters_;
    BasicBlock *entry_;
}; // class Function

class BasicBlock : public Node {
public:
    
}; // class BasicBlock


class Value : public Node {
public:
    static Value *New0(base::Arena *arena, Type type, Operator *op) {
        //return new (arena) Value(arena, type, op);
        auto chunk = arena->Allocate(RequiredSize(op));
        return new (chunk) Value(arena, type, op);
    }
    
    template<class ...Nodes>
    static inline Value *New(base::Arena *arena, Type type, Operator *op, Nodes... nodes) {
        Node *inputs[] = {CheckNode(nodes)...};
        return NewWithInputs(arena, type, op, inputs, sizeof(inputs)/sizeof(inputs[0]));
    }
    
    static Value *NewWithInputs(base::Arena *arena, Type type, Operator *op, Node **inputs, size_t size);
    
    DEF_PTR_PROP_RW(Operator, op);
    
    Value *InputValue(int i) const {
        assert(i >= 0 && i < op()->value_in());
        return DCHECK_NOTNULL(io_[value_in_offset() + i]->AsValue());
    }
    
    Value *OutputValue(int i) {
        assert(i >= 0 && i < op()->value_out());
        return i == 0 ? this : OverflowOutputValue(i);
    }
    
    Value *OverflowOutputValue(int i) const {
        assert(i > 0 && i < op()->value_out());
        return DCHECK_NOTNULL(io_[overflow_value_out_offset() + i - 1]->AsValue());
    }
    
    BasicBlock *InputControl(int i) const {
        assert(i >= 0 && i < op()->control_in());
        return DCHECK_NOTNULL(io_[control_in_offset() + i]->AsBasicBlock());
    }
    
    BasicBlock *OutputControl(int i) const {
        assert(i >= 0 && i < op()->control_out());
        return DCHECK_NOTNULL(io_[control_out_offset() + i]->AsBasicBlock());
    }
    
    struct User {
        User *prev;
        User *next;
        Value *user;
        int position;
    }; // class User
    
    User *AddUser(base::Arena *arena, Value *user, int position);
    User *FindUser(Value *user);
    User *FindUser(Value *user, int position);
private:
    Value(base::Arena *arena, Type type, Operator *op);
    
    void *operator new (size_t /*n*/, void *chunk) { return chunk; }
    static Node *CheckNode(Node *node) { return DCHECK_NOTNULL(node); }
    
    static size_t RequiredSize(const Operator *op) {
        return sizeof(Value) + (TotalInOutputs(op)) * sizeof(Node *);
    }
    
    static size_t TotalInOutputs(const Operator *op) {
        const int overflow_value_out = op->value_out() == 0 ? 0 : op->value_out() - 1;
        return op->value_in() + overflow_value_out + op->control_in() + op->control_out();
    }
    
    constexpr int value_in_offset() const { return 0; }
    int overflow_value_out_offset() const { return value_in_offset() + op()->value_in(); }
    int control_in_offset() const { return overflow_value_out_offset() + op()->value_out() - 1; }
    int control_out_offset() const { return control_in_offset() + op()->control_in(); }

    Type type_;
    Operator *op_;
    uint32_t has_users_overflow_: 1;
    uint32_t padding0_: 3;
    uint32_t users_size_: 14;
    uint32_t inline_users_capacity_: 14;
    User *inline_users_; // users array or linked-list header
    User  overflow_users_;
    
    //uint32_t is_inline_io_;
    // input[0] input[1] ...| output[1] output[2] ...
    Node *io_[1]; // inputs + overflow outputs
}; // class Value


#define DEFINE_METHODS(name) \
    inline name *Node::As##name() { return (!this || kind_ != k##name) ? nullptr : static_cast<name *>(this); } \
    inline const name *Node::As##name() const { \
        return (!this || kind_ != k##name) ? nullptr : static_cast<const name *>(this); \
    }
    DECLARE_IR_KINDS(DEFINE_METHODS)
#undef DEFINE_METHODS


template<class T>
inline T OperatorWith<T>::Data(const ir::Value *node) { return Cast(node->op())->data(); }

} // namespace ir

} // namespace yalx

#endif // YALX_IR_NODE_H_
