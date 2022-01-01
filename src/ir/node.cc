#include "ir/node.h"
#include "ir/metadata.h"
#include "base/format.h"
#include <string.h>

namespace yalx {

namespace ir {

Module::Module(base::Arena *arena, const String *name, const String *full_name, const String *path, const String *full_path)
    : Node(Node::kModule)
    , arena_(DCHECK_NOTNULL(arena))
    , name_(DCHECK_NOTNULL(name))
    , full_name_(DCHECK_NOTNULL(full_name))
    , path_(DCHECK_NOTNULL(path))
    , full_path_(DCHECK_NOTNULL(full_path))
    , named_models_(arena)
    , interfaces_(arena)
    , structures_(arena)
    , global_values_(arena)
    , funs_(arena)
    , values_(arena)
    , source_position_table_(arena) {
}

InterfaceModel *Module::NewInterfaceModel(const String *name, const String *full_name) {
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    auto model = new (arena()) InterfaceModel(arena(), name, full_name);
    named_models_[name->ToSlice()] = model;
    interfaces_.push_back(model);
    return model;
}

StructureModel *Module::NewClassModel(const String *name, const String *full_name, StructureModel *base_of) {
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    auto clazz = new (arena()) StructureModel(arena(), name, full_name, StructureModel::kClass, this, base_of);
    named_models_[name->ToSlice()] = clazz;
    structures_.push_back(clazz);
    return clazz;
}

StructureModel *Module::NewStructModel(const String *name, const String *full_name, StructureModel *base_of) {
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    assert(named_models_.find(name->ToSlice()) == named_models_.end());
    auto clazz = new (arena()) StructureModel(arena(), name, full_name, StructureModel::kStruct, this, base_of);
    named_models_[name->ToSlice()] = clazz;
    structures_.push_back(clazz);
    return clazz;
}

Function *Module::NewFunction(const String *name, StructureModel *owns, PrototypeModel *prototype) {
    auto full_name = String::New(arena(), base::Sprintf("%s.%s.%s", name_->data(), owns->name()->data(), name->data()));
    return NewFunction(full_name, full_name, prototype);
}

Function *Module::NewFunction(const String *name, const String *full_name, PrototypeModel *prototype) {
    auto fun = new (arena_) Function(arena_, name, full_name, this, prototype);
    assert(global_values_.find(name->ToSlice()) == global_values_.end());
    global_values_[name->ToSlice()] = GlobalSlot{funs_size(), true};
    funs_.push_back(fun);
    return fun;
}

Function *Module::NewFunction(PrototypeModel *prototype) {
    auto random_name = String::New(arena_, base::Sprintf("$unnamed$_%d", (rand() << 4) | next_unnamed_id_++));
    auto fun = NewFunction(random_name, random_name, prototype);
    return fun;
}

Function *Module::NewStandaloneFunction(const String *name, PrototypeModel *prototype) {
    return new (arena_) Function(arena_, name, name, this, prototype);
}

//Value *Module::NewGlobalValue(SourcePosition source_position, const String *name, Type type) {
//    assert(global_values_.find(name->ToSlice()) == global_values_.end());
//    
//}


Function *Module::FindFunOrNull(std::string_view name) const {
    auto iter = global_values_.find(name);
    if (iter == global_values_.end()) {
        return nullptr;
    }
    return !iter->second.fun_or_val ? nullptr : fun(iter->second.offset);
}

Value *Module::FindValOrNull(std::string_view name) const {
    auto iter = global_values_.find(name);
    if (iter == global_values_.end()) {
        return nullptr;
    }
    return iter->second.fun_or_val ? nullptr : value(iter->second.offset);
}

BasicBlock *Function::NewBlock(const String *name) {
    auto block = new (arena_) BasicBlock(arena_, name);
    if (!entry_) {
        entry_ = block;
    }
    blocks_.push_back(block);
    return block;
}

Function::Function(base::Arena *arena, const String *name, const String *full_name, Module *owns, PrototypeModel *prototype)
    : Node(Node::kFunction)
    , arena_(DCHECK_NOTNULL(arena))
    , name_(DCHECK_NOTNULL(name))
    , full_name_(DCHECK_NOTNULL(full_name))
    , owns_(DCHECK_NOTNULL(owns))
    , prototype_(prototype)
    , paramaters_(arena)
    , blocks_(arena) {
}

BasicBlock::BasicBlock(base::Arena *arena, const String *name)
    : Node(Node::kBasicBlock)
    , arena_(DCHECK_NOTNULL(arena))
    , name_(DCHECK_NOTNULL(name))
    , instructions_(arena)
    , inputs_(arena)
    , outputs_(arena) {
}

Value *Value::NewWithInputs(base::Arena *arena, const String *name, SourcePosition source_position, Type type,
                            Operator *op, Node **inputs, size_t size) {
    auto value = New0(arena, name, source_position, type, op);
    assert(size == TotalInOutputs(op));
    ::memcpy(value->io_, inputs, size * sizeof(Node *));
    for (size_t i = 0; i < size; i++) {
        if (inputs[i]->IsValue()){
            inputs[i]->AsValue()->AddUser(arena, value, static_cast<int>(i));
        }
    }
    return value;
}

Value::Value(base::Arena *arena, const String *name, SourcePosition source_position, Type type, Operator *op)
    : Node(Node::kValue)
    , name_(name)
    , source_position_(source_position)
    , type_(type)
    , op_(DCHECK_NOTNULL(op))
    , has_users_overflow_(0)
    , users_size_(0)
    , inline_users_capacity_(0)
    , inline_users_(nullptr) {
    overflow_users_.next = &overflow_users_;
    overflow_users_.prev = &overflow_users_;
        
#ifdef DEBUG
    ::memset(io_, 0, TotalInOutputs(op) * sizeof(io_[0]));
#endif
}

Value::User *Value::AddUser(base::Arena *arena, Value *user, int position) {
    if (!inline_users_) {
        assert(!has_users_overflow_);
        inline_users_capacity_ = 8;
        inline_users_ = arena->NewArray<User>(inline_users_capacity_);
    }
    if (auto found = FindUser(user, position); found) {
        return found;
    }
    
    if (!has_users_overflow_) {
        if (users_size_ + 1 >= inline_users_capacity_) {
            has_users_overflow_ = 0;
        }
    }
    
    if (has_users_overflow_) {
        auto node = arena->New<User>();
        node->next = node;
        node->prev = node;
        node->user = user;
        node->position = position;
        QUEUE_INSERT_TAIL(&overflow_users_, node);
        users_size_ ++;
        return node;
    } else {
        auto node = &inline_users_[users_size_++];
        node->user = user;
        node->position = position;
        users_size_ ++;
        return node;
    }
}

Value::User *Value::FindUser(Value *user, int position) {
    for (int i = 0; std::min(users_size_, inline_users_capacity_); i++) {
        if (inline_users_[i].user == user && inline_users_[i].position == position) {
            return &inline_users_[i];
        }
    }
    for (auto node = overflow_users_.next; node != &overflow_users_; node = node->next) {
        if (node->user == user && node->position == position) {
            return node;
        }
    }
    return nullptr;
}

} // namespace ir

} // namespace yalx
