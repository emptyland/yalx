#pragma once
#ifndef YALX_IR_NODE_H_
#define YALX_IR_NODE_H_

#include "ir/operator.h"
#include "ir/type.h"
#include "ir/source-position.h"
#include "base/checking.h"
#include "base/arena-utils.h"
#include "base/arena.h"
#include <algorithm>

namespace yalx {
namespace base {
class ArenaString;
class PrintingWriter;
} // namespace base
namespace ir {

using String = base::ArenaString;

#define DECLARE_IR_KINDS(V) \
    V(Module) \
    V(Function) \
    V(BasicBlock) \
    V(Value)

#define DEFINE_PREDECL(name) class name;
    DECLARE_IR_KINDS(DEFINE_PREDECL)
#undef DEFINE_PREDECL

class PrintingContext;
class Model;
class PrototypeModel;
class InterfaceModel;
class ArrayModel;
class ChannelModel;
class StructureModel;

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


class Function : public Node {
public:
    enum Decoration {
        kDefault,
        kNative,
        kAbstract,
        kOverride,
    };
    
    constexpr static const uint32_t kNeverReturnBit = 1u;
    constexpr static const uint32_t kNativeHandleBit = 1u << 1;
    constexpr static const uint32_t kUnwindHandleBit = 1u << 2;
    
    BasicBlock *NewBlock(const String *name);
    
    DEF_PTR_GETTER(const String, name);
    DEF_PTR_GETTER(const String, full_name);
    DEF_PTR_GETTER(Module, owns);
    DEF_PTR_GETTER(base::Arena, arena);
    DEF_PTR_GETTER(BasicBlock, entry);
    DEF_PTR_GETTER(PrototypeModel, prototype);
    DEF_VAL_GETTER(Decoration, decoration);
    DEF_PTR_PROP_RW(const String, native_stub_name);
    DEF_ARENA_VECTOR_GETTER(Value *, paramater);
    DEF_ARENA_VECTOR_GETTER(BasicBlock *, block);
    
    bool is_never_return() const { return properties_ & kNeverReturnBit; }
    bool is_native_handle() const { return properties_ & kNativeHandleBit; }
    bool should_unwind_handle() const { return properties_ & kUnwindHandleBit; }
    
    void AddPropertiesBits(uint32_t bits) { properties_ |= bits; }
    
    void SetPropertiesBits(uint32_t bits) {
        properties_ &= ~bits;
        properties_ |= bits;
    }
    
    void MoveToLast(BasicBlock *blk) {
        if (auto iter = std::find(blocks_.begin(), blocks_.end(), blk); iter != blocks_.end()) {
            blocks_.erase(iter);
            blocks_.push_back(blk);
        }
    }
    
    void MoveToAfterOf(BasicBlock *after, BasicBlock *before) {
        if (auto iter = std::find(blocks_.begin(), blocks_.end(), before); iter != blocks_.end()) {
            blocks_.erase(iter);
        }
        if (auto iter = std::find(blocks_.begin(), blocks_.end(), after); iter != blocks_.end()) {
            blocks_.insert(iter + 1, before);
        }
    }
    
    void ReplaceUsers(const std::map<Value *, Value *> &values, BasicBlock *begin, BasicBlock *end);

    inline void UpdateIdsOfBlocks();
    
    void PrintTo(PrintingContext *ctx, base::PrintingWriter *printer, Model *owns = nullptr) const;
    friend class Module;
private:
    Function(base::Arena *arena, const Decoration decoration, const String *name, const String *full_name, Module *owns,
             PrototypeModel *prototype);
    
    const String *const name_;
    const String *const full_name_;
    Module *const owns_;
    base::Arena *const arena_;
    PrototypeModel *const prototype_;
    Decoration const decoration_;
    uint32_t properties_ = 0;
    const String *native_stub_name_ = nullptr;
    BasicBlock *entry_ = nullptr;
    base::ArenaVector<Value *> paramaters_;
    base::ArenaVector<BasicBlock *> blocks_;
}; // class Function

class Module : public Node {
public:
    Module(base::Arena *arena, const String *name, const String *full_name, const String *path, const String *full_path);

    DEF_PTR_GETTER(const String, name);
    DEF_PTR_GETTER(base::Arena, arena);
    DEF_PTR_GETTER(const String, path);
    DEF_PTR_GETTER(const String, full_name);
    DEF_PTR_GETTER(const String, full_path);
    DEF_ARENA_VECTOR_GETTER(Function *, fun);
    DEF_ARENA_VECTOR_GETTER(Value *, value);
    DEF_ARENA_VECTOR_GETTER(InterfaceModel *, interface);
    DEF_ARENA_VECTOR_GETTER(StructureModel *, structure);
    DEF_VAL_GETTER(SourcePositionTable, source_position_table);
    DEF_VAL_MUTABLE_GETTER(SourcePositionTable, source_position_table);

    Function *NewFunction(const Function::Decoration decoration, const String *name, StructureModel *owns,
                          PrototypeModel *prototype);
    Function *NewFunction(const Function::Decoration decoration, const String *name, const String *full_name,
                          PrototypeModel *prototype);
    Function *NewFunction(PrototypeModel *prototype);
    Function *NewStandaloneFunction(const Function::Decoration decoration, const String *name, const String *full_name,
                                    PrototypeModel *prototype);
    
    InterfaceModel *NewInterfaceModel(const String *name, const String *full_name);
    StructureModel *NewClassModel(const String *name, const String *full_name, StructureModel *base_of);
    StructureModel *NewStructModel(const String *name, const String *full_name, StructureModel *base_of);
    StructureModel *NewEnumModel(const String *name, const String *full_name);

    void InsertGlobalValue(const String *name, Value *value) {
        assert(global_values_.find(name->ToSlice()) == global_values_.end());
        global_values_[name->ToSlice()] = GlobalSlot { values_size(), false };
        values_.push_back(value);
    }
    
    Function *FindFunOrNull(std::string_view name) const;
    Value *FindValOrNull(std::string_view name) const;
    
    Model *FindModelOrNull(std::string_view name) const {
        auto iter = named_models_.find(name);
        return iter == named_models_.end() ? nullptr : iter->second;
    }
    
    void PrintTo(base::PrintingWriter *printer) const;
private:
    struct GlobalSlot {
        size_t offset;
        bool fun_or_val;
    };
    
    const String *const name_;
    const String *const path_;
    const String *const full_name_;
    const String *const full_path_;
    base::Arena *const arena_;
    
    base::ArenaMap<std::string_view, Model *> named_models_;
    base::ArenaVector<InterfaceModel *> interfaces_;
    base::ArenaVector<StructureModel *> structures_;
    
    base::ArenaMap<std::string_view, GlobalSlot> global_values_;
    base::ArenaVector<Function *> funs_;
    base::ArenaVector<Value *> values_;
    
    SourcePositionTable source_position_table_;
    int next_unnamed_id_ = 0;
}; // class Module


class BasicBlock : public Node {
public:
    struct PhiUser {
        Value *phi;
        Value *dest;
    };
    
    template<class ...Nodes>
    inline Value *NewNode(SourcePosition source_position, Type type, Operator *op, Nodes... nodes);
    
    template<class ...Nodes>
    inline Value *NewNode(const String *name, SourcePosition source_position, Type type, Operator *op, Nodes... nodes);
    
    inline Value *NewNodeWithValues(const String *name, SourcePosition source_position, Type type, Operator *op,
                                    const std::vector<Value *> &values);
    
    inline Value *NewNodeWithValues(const String *name, SourcePosition source_position, Type type, Operator *op,
                                    Value **values, size_t values_size);
    
    inline Value *NewNodeWithNodes(const String *name, SourcePosition source_position, Type type, Operator *op,
                                   const std::vector<Node *> &nodes);
    
    inline Value *NewNodeWithNodes(const String *name, SourcePosition source_position, Type type, Operator *op,
                                   Node **nodes, size_t nodes_size);
    
    DEF_PTR_GETTER(const String, name);
    DEF_PTR_GETTER(base::Arena, arena);
    DEF_VAL_GETTER(int, id);
    DEF_ARENA_VECTOR_GETTER(Value *, instruction);
    DEF_ARENA_VECTOR_GETTER(PhiUser, phi_node_user);
    DEF_ARENA_VECTOR_GETTER(BasicBlock *, input);
    DEF_ARENA_VECTOR_GETTER(BasicBlock *, output);
    
    void LinkTo(BasicBlock *output) {
        if (output->FindInput(this) < 0) { output->inputs_.push_back(this); }
        if (FindOutput(output) < 0) { outputs_.push_back(output); }
    }
    
    int FindInput(BasicBlock *node) {
        auto iter = std::find(inputs_.begin(), inputs_.end(), node);
        return iter == inputs_.end() ? -1 : static_cast<int>(iter - inputs_.begin());
    }
    
    int FindOutput(BasicBlock *node) {
        auto iter = std::find(outputs_.begin(), outputs_.end(), node);
        return iter == outputs_.end() ? -1 : static_cast<int>(iter - outputs_.begin());
    }
    
    void MoveToFront(Value *instr) {
        if (auto iter = std::find(instructions_.begin(), instructions_.end(), instr); iter != instructions_.end()) {
            instructions_.erase(iter);
            instructions_.insert(instructions_.begin(), instr);
        }
    }
    
    void RemoveDeads();
    void RemovePhiUsersOfDeads();
    
    void AddInstruction(Value *value);
    
    int FindInstruction(Value *value) const {
        auto it = std::find(instructions_.begin(), instructions_.end(), value);
        return it == instructions_.end() ? -1 : static_cast<int>(it - instructions_.begin());
    }
    
    void PrintTo(PrintingContext *ctx, base::PrintingWriter *printer) const;
    friend class Function;
private:
    BasicBlock(base::Arena *arena, const String *name);
    
    void RemovePhiNodeUser(Value *phi) {
        auto iter = std::find_if(phi_node_users_.begin(), phi_node_users_.end(),
                                 [phi](auto user){ return user.phi == phi; });
        phi_node_users_.erase(iter);
    }
    
    const String *const name_;
    base::Arena *const arena_;
    int id_ = -1;
    base::ArenaVector<Value *> instructions_;
    base::ArenaVector<BasicBlock *> inputs_;
    base::ArenaVector<BasicBlock *> outputs_;
    base::ArenaVector<PhiUser> phi_node_users_;
}; // class BasicBlock


class Value : public Node {
public:
    static Value *New0(base::Arena *arena, SourcePosition source_position, Type type, Operator *op) {
        return New0(arena, nullptr, source_position, type, op);
    }
    
    static Value *New0(base::Arena *arena, const String *name, SourcePosition source_position, Type type, Operator *op) {
        auto chunk = arena->Allocate(RequiredSize(op));
        return new (chunk) Value(arena, name, source_position, type, op);
    }
    
    template<class ...Nodes>
    static inline Value *New(base::Arena *arena, SourcePosition source_position, Type type, Operator *op, Nodes... nodes) {
        Node *inputs[] = {CheckNode(nodes)...};
        return NewWithInputs(arena, nullptr, source_position, type, op, inputs, sizeof(inputs)/sizeof(inputs[0]));
    }
    
    template<class ...Nodes>
    static inline Value *New(base::Arena *arena, const String *name, SourcePosition source_position, Type type,
                                  Operator *op, Nodes... nodes) {
        Node *inputs[] = {CheckNode(nodes)...};
        return NewWithInputs(arena, name, source_position, type, op, inputs, sizeof(inputs)/sizeof(inputs[0]));
    }
    
    static Value *NewWithInputs(base::Arena *arena, const String *name, SourcePosition source_position, Type type,
                                Operator *op, Node **inputs, size_t size);
    
    DEF_PTR_PROP_RW(const String, name);
    DEF_PTR_PROP_RW(Operator, op);
    DEF_VAL_GETTER(Type, type);
    DEF_VAL_GETTER(SourcePosition, source_position);
    
    bool IsNot(Operator::Value value) const { return !Is(value); }
    bool Is(Operator::Value value) const { return op()->value() == value; }
    
    Value *InputValue(int i) const {
        assert(i >= 0 && i < op()->value_in());
        return DCHECK_NOTNULL(io_[value_in_offset() + i]->AsValue());
    }
    
    Value *OutputValue(int i) const {
        assert(i >= 0 && i < op()->value_out());
        return i == 0 ? const_cast<Value *>(this) : OverflowOutputValue(i);
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
    
    void SetOutputControl(int i, BasicBlock *node) {
        assert(i >= 0 && i < op()->control_out());
        io_[control_out_offset() + i] = DCHECK_NOTNULL(node);
    }
    
    void Kill() {
        assert(IsAlive());
        for (size_t i = 0; i < TotalInOutputs(op()); i++) { io_[i] = nullptr; }
        op_ = nullptr;
    }
    
    bool IsDead() const { return op_ == nullptr; }
    bool IsAlive() const { return !IsDead(); }
    
    void Replace(base::Arena *arena, int position, Value *from, Value *to);
    
    struct User {
        User  *prev;
        User  *next;
        Value *user;
        int    position;
    }; // class User
    
    User *AddUser(base::Arena *arena, Value *user, int position);
    User *FindUser(Value *user);
    User *FindUser(Value *user, int position);
    void RemoveUser(User *user);
    
    class Users {
    public:
        class iterator {
        public:
            iterator(Users *owns, int inline_index, User *overflow_node)
            : owns_(owns)
            , inline_index_(inline_index)
            , overflow_node_(overflow_node) {}
            
            User *operator -> () { return get(); }
            User &operator * () { return *get(); }
            void operator ++ (int) { next(); }
            void operator ++ () { next(); }
            
            bool operator != (const iterator &other) const { return !operator == (other); }
            
            bool operator == (const iterator &other) const {
                if (owns_ != other.owns_) {
                    return false;
                }
                return inline_index_ == other.inline_index_ && overflow_node_ == other.overflow_node_;
            }

            iterator &operator = (const iterator &) = default;
            
        private:
            User *get() {
                if (overflow_node_) { return overflow_node_; }
                else { return &owns_->inline_users_[inline_index_]; }
            }
            
            void next() {
                if (inline_index_ < owns_->inline_users_capacity_ - 1) {
                    inline_index_++;
                } else if (!overflow_node_) {
                    overflow_node_ = owns_->overflow_users_->next;
                } else {
                    overflow_node_ = overflow_node_->next;
                }
            }
            
            Users *owns_;
            int inline_index_ = 0;
            User *overflow_node_ = nullptr;
        };
        
        Users(Value *owns);
        
        uint32_t size() const { return users_size_; }
        
        iterator begin() { return iterator{this, 0, nullptr}; }
        iterator end() { return iterator{this, last_index(), last_node()}; }
    private:
        int last_index() const {
            return std::min(static_cast<int>(users_size_), static_cast<int>(inline_users_capacity_) - 1) ;
        }

        User *last_node() { return has_overflow() ? overflow_users_ : nullptr; }
        
        bool has_overflow() const {
            bool has = users_size_ > inline_users_capacity_;
            if (has) {
                assert(overflow_users_->next != overflow_users_);
                assert(overflow_users_->prev != overflow_users_);
            }
            return has;
        }
        
        bool has_users_overflow_;
        uint32_t users_size_;
        uint32_t inline_users_capacity_;
        User *inline_users_; // users array or linked-list header
        User *overflow_users_;
    }; // class Value::Users
    
    Users users() { return Users(this); }
    
    std::vector<std::tuple<int, Value *>> GetUsers(){
        std::vector<std::tuple<int, Value *>> target;
        if (users_size_ == 0) {
            return target;
        }
        for (auto edge : users()) {
            target.push_back(std::make_tuple(edge.position, edge.user));
        }
        return target;
    }
    
    void PrintTo(PrintingContext *ctx, base::PrintingWriter *printer) const;
    void IfConstantPrintTo(PrintingContext *ctx, base::PrintingWriter *printer) const;
private:
    Value(base::Arena *arena, const String *name, SourcePosition source_position, Type type, Operator *op);
    
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

    const String *name_;
    Type type_;
    Operator *op_;
    SourcePosition source_position_;
    uint32_t has_users_overflow_: 1;
    uint32_t padding0_: 3;
    uint32_t users_size_: 14;
    uint32_t inline_users_capacity_: 14;
    User *inline_users_; // users array
    User  overflow_users_; // users linked-list header
    
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


inline void Function::UpdateIdsOfBlocks() {
    int id = 0;
    for (auto blk : blocks()) {
        blk->id_ = id++;
    }
}

template<class T>
inline T OperatorWith<T>::Data(const ir::Value *node) { return Cast(node->op())->data(); }

template<class ...Nodes>
inline Value *BasicBlock::NewNode(SourcePosition source_position, Type type, Operator *op, Nodes... nodes) {
    Node *inputs[] = {DCHECK_NOTNULL(nodes)...};
    auto instr = Value::NewWithInputs(arena(), nullptr, source_position, type, op, inputs,
                                      sizeof(inputs)/sizeof(inputs[0]));
    AddInstruction(instr);
    return instr;
}

template<class ...Nodes>
inline Value *BasicBlock::NewNode(const String *name, SourcePosition source_position, Type type, Operator *op,
                                  Nodes... nodes) {
    Node *inputs[] = {DCHECK_NOTNULL(nodes)...};
    auto instr = Value::NewWithInputs(arena(), name, source_position, type, op, inputs, sizeof(inputs)/sizeof(inputs[0]));
    AddInstruction(instr);
    return instr;
}

inline Value *BasicBlock::NewNodeWithValues(const String *name, SourcePosition source_position, Type type, Operator *op,
                                            const std::vector<Value *> &inputs) {
    return NewNodeWithValues(name, source_position, type, op, const_cast<Value **>(&inputs[0]), inputs.size());
}

inline Value *BasicBlock::NewNodeWithValues(const String *name, SourcePosition source_position, Type type, Operator *op,
                                            Value **inputs, size_t inputs_size) {
    auto instr = Value::NewWithInputs(arena(), name, source_position, type, op, reinterpret_cast<Node **>(inputs),
                                      inputs_size);
    AddInstruction(instr);
    return instr;
}

inline Value *BasicBlock::NewNodeWithNodes(const String *name, SourcePosition source_position, Type type, Operator *op,
                                           const std::vector<Node *> &nodes) {
    return NewNodeWithNodes(name, source_position, type, op, const_cast<Node **>(&nodes[0]), nodes.size());
}

inline Value *BasicBlock::NewNodeWithNodes(const String *name, SourcePosition source_position, Type type, Operator *op,
                                           Node **nodes, size_t nodes_size)  {
    auto instr = Value::NewWithInputs(arena(), name, source_position, type, op, nodes, nodes_size);
    AddInstruction(instr);
    return instr;
}

} // namespace ir

} // namespace yalx

#endif // YALX_IR_NODE_H_
